import os
import re
from datetime import datetime
from fat import FAT
from bitmap import Bitmap
from blocks import BlockList, Reader, Writer
from files import File, Directory
from system_constants import BLOCK_LIST_IDX, TOTAL_BLOCKS, FINAL_BLOCK
from patterns import BLOCK_START

file_system = None
class FileSystem:
  """ valores acessíveis:
  filename: nome do arquivo da unidade
  fat: instância da FAT na memória
  bitmap: instância do BITMAP na memória """
  def mount(self, filename):
    self.filename = filename
    self.load_filesystem()
    print(f'Arquivo {filename} montado')
    self.mounted = True

  def umount(self):
    if (not self.mounted):
      print('Erro: nenhum arquivo montado')
      return
    print(f'Desmontando {self.filename}')
    w = Writer(self)
    w.write_fat()
    w.write_bitmap()
    print(f'tabela fat e bitmap escritos no arquivo {self.filename}')
    self.fat = None
    self.bitmap = None
    self.mounted = False
  
  # carrega a unidade ou cria uma nova caso ela não exista em 'filename'
  def load_filesystem(self):
    is_new = False
    if (not os.path.isfile(self.filename)):
      self.create_filesystem()
      is_new = True
    self.fat = FAT(self.filename)
    self.bitmap = Bitmap(self.filename)
    BlockList(self.filename, is_new)

  # inicializa a unidade totalmente vazia
  def create_filesystem(self):
    if ('/' in self.filename):
      os.makedirs(os.path.dirname(self.filename), exist_ok=True)
    fp = open(self.filename, 'w')
    fp.close()
  
  def nearest_empty_block(self, block):
    step = 1
    if (block == TOTAL_BLOCKS):
      step = -1
    i = block + step
    while (self.bitmap.map[i] != 1):
      i += step
    return i
  
  """ Recebe o bloco do diretório e o objeto do arquivo e cria uma entrada na
  tabela do diretório para o arquivo dado """
  def create_file_entry(self, file):
    filename = file.name.split('/')[-1]
    size = '{:07x}'.format(file.size)
    first_block = '{:04x}'.format(file.first_block)
    file_entry = f'^{filename}&{size}&{file.created_at}&{file.accessed_at}&{file.modified_at}&{first_block}|'
    return file_entry
  
  def write_file_to_unit(self, dir_block, file):
    entry = self.create_file_entry(file)
    self.fat.table[file.first_block] = FINAL_BLOCK
    self.bitmap.map[file.first_block] = 0
    w = Writer(self)
    w.write_file(dir_block, entry, file)
  
  """ Cria uma nova entrada para um arquivo existente e substituí a entrada
  antiga """
  def update_file_entry(self, block, file):
    new_entry = self.create_file_entry(file)
    w = Writer(self)
    w.update_entry(file, block, new_entry)
  
  """ Cria uma nova entrada para um diretório existente e substituí a entrada
  antiga """
  def update_dir_entry(self, block, dir):
    new_entry = self.create_dir_entry(dir)
    w = Writer(self)
    w.update_entry(dir, block, new_entry)

  """ Recebe o bloco do diretório e o objeto do arquivo e cria uma entrada na
  tabela do diretório para o arquivo dado """
  def create_dir_entry(self, dir):
    dirname = dir.name
    first_block = '{:04x}'.format(dir.first_block)
    dir_entry = f'%{dirname}&{dir.created_at}&{dir.accessed_at}&{dir.modified_at}&{first_block}|'
    return dir_entry
  
  def write_dir_to_unit(self, dir_block, dir):
    entry = self.create_dir_entry(dir)
    self.fat.table[dir.first_block] = FINAL_BLOCK
    self.bitmap.map[dir.first_block] = 0
    w = Writer(self)
    w.write_directory(dir_block, entry)

class CP:
  
  def __init__(self, origin, destiny, fs):
    self.origin = origin
    self.destiny = destiny
    self.destiny_name = self._get_destiny_name()
    self.destiny_path = self._get_destiny_path()
    self.fs = fs
    self.content = self.read_origin_content()
  
  def read_origin_content(self):
    f = open(self.origin, 'r')
    return f.read()
  
  def _get_destiny_path(self):
    last_slash = self.destiny.rfind('/')
    return self.destiny[:last_slash]
  
  def _get_destiny_name(self):
    return self.destiny.split('/')[-1]
  
  def cp(self):
    # inicia o objeto de leitura
    r = Reader(self.fs)
    # lê o conteúdo da root
    files, _, block = r.read_path(self.destiny_path)
    # verifica se o diretório de destino existe
    if block == -1:
      print(f'Erro: diretório {self.destiny_path} não existe!'),
      return
    # verifica se um arquivo com o mesmo nome já existe no diretório e o apaga caso exista
    self.path_block = block
    self.remove_duplicates(files)
    block_content = r.raw_block_content(block)
    block_content = re.sub(BLOCK_START, block_content, '')
    destiny = self.destiny_file()
    # se o bloco não estiver cheio
    if (len(block_content) != 4096):
      self.fs.write_file_to_unit(block, destiny)
  
  def destiny_file(self):
    first_block = self.fs.nearest_empty_block(self.path_block)
    timestamp = int(datetime.now().timestamp())
    file = File(self.destiny, 0, timestamp, timestamp, timestamp, first_block)
    file.set_content(self.content)
    return file  
  def remove_duplicates(self, files):
    for file in files:
      if file.name == self.destiny_name:
        w = Writer(self.fs)
        w.erase_file(self.path_block, file)
        break

class Touch:

  def __init__(self, filepath, fs):
    self.filepath = filepath
    self.parent_dir = self._get_parent_dir()
    self.filename = self._get_filename()
    self.fs = fs

  def _get_parent_dir(self):
    last_slash = self.filepath.rfind('/')
    return self.filepath[:last_slash]

  def _get_filename(self):
    last_slash = self.filepath.rfind('/')
    return self.filepath[last_slash + 1:]

  """ Cria um arquivo vazio em filename ou atualiza a data de acesso do arquivo
  caso ele já exista """
  def touch(self):
    timestamp = int(datetime.now().timestamp())
    r = Reader(self.fs)
    files, dirs, parent_block = r.read_path(self.parent_dir)
    if parent_block == -1:
      print(f'Erro: {self.parent_dir} não existe')
      return

    for file in files:
      if file.name == self.filename:
        # atualiza o accessed_at do arquivo
        file.accessed_at = timestamp
        file.modified_at = timestamp
        self.fs.update_file_entry(parent_block, file)
        return
    
    for dir in dirs:
      if dir.name == self.filename:
        # atualiza o accessed_at do diretório
        dir.accessed_at = timestamp
        dir.modified_at = timestamp
        self.fs.update_dir_entry(parent_block, dir)
        return

    first_block = self.fs.nearest_empty_block(parent_block)
    file = File(self.filename, 0, timestamp, timestamp, timestamp, first_block)
    file.set_content('')
    self.fs.write_file_to_unit(parent_block, file)

class MKDIR:

  def __init__(self, dirpath, fs):
    self.dirpath = dirpath
    self.parent_dir = self._get_parent_dir()
    self.dirname = self._get_dirname()
    self.fs = fs

  def _get_parent_dir(self):
    last_slash = self.dirpath.rfind('/')
    return self.dirpath[:last_slash]
  
  def _get_dirname(self):
    last_slash = self.dirpath.rfind('/')
    return self.dirpath[last_slash + 1:]
  
  """ Cria um diretório vazio """
  def mkdir(self):
    # inicia o objeto de leitura
    r = Reader(self.fs)
    # lê o conteúdo da root
    files, dirs, parent_block = r.read_path(self.parent_dir)
    # encontra o timestamp
    timestamp = int(datetime.now().timestamp())
    # verifica se "destino" existe
    if parent_block == -1:
      print(f'Erro: diretório {self.parent_dir} não existe!'),
      return
    #checa se há algum diretório com o mesmo nome
    for dir in dirs:
      if dir.name == self.dirname:
        print(f'Erro: diretório {self.parent_dir} já existe! Não é possível criar um diretório com o mesmo nome'),
        return
    #checa se há algum arquivo com o mesmo nome
    for file in files:
      if file.name == self.dirname:
        print(f'Erro: arquivo {self.parent_dir} já existe! Não é possível criar um diretório com o mesmo nome'),
        return
    first_block = self.fs.nearest_empty_block(parent_block)
    dir = Directory(self.dirname, timestamp, timestamp, timestamp, first_block)
    self.fs.write_dir_to_unit(parent_block, dir)


class CAT:

  def __init__(self, path, fs):
    self.path = path
    self.fs = fs
    self.parent_dir = self._get_parent_dir()
    self.file_name = self._get_filename()
  
  def _get_parent_dir(self):
    last_slash = self.path.rfind('/')
    return self.path[:last_slash]
  
  def _get_filename(self):
    last_slash = self.path.rfind('/')
    return self.path[last_slash + 1:]
  
  def cat(self):
    file = self.locate_file()
    if not file:
      print(f'Erro: arquivo regular {self.file_name} não encontrado')
      return
    r = Reader(self.fs)
    r.print_file_content(file)
    print()
  
  def locate_file(self):
    r = Reader(self.fs)
    f = None
    files, _, _ = r.read_path(self.parent_dir)
    for file in files:
      if file.name == self.file_name:
        f =  file
        break
    return f

class LS:

  def __init__(self, path, fs):
    self.path = path
    self.fs = fs
  
  def ls(self):
    files, dirs, block = self.get_path_content()
    if (block == -1):
      print(f'Erro: não há diretório em {self.path}')
      return
    files.extend(dirs)
    files.sort(key=self.by_name)
    self.print_output(files)
  
  def print_output(self, path_content):
    for content in path_content:
      if content.__class__ == File:
        self.print_file_info(content)
      elif content.__class__ == Directory:
        self.print_dir_info(content)
  
  def print_file_info(self, file):
    print(f'ARQUIVO   {self.timestring(file.modified_at)} {file.name} {file.size} B')
  
  def print_dir_info(self, dir):
    print(f'DIRETÓRIO {self.timestring(dir.modified_at)} {dir.name}')
  
  def timestring(self, timestamp):
    modified_at = datetime.fromtimestamp(timestamp)
    return modified_at.strftime("%d %B %Y %H:%M:%S")
  
  def by_name(self, obj):
    return obj.name
  
  def get_path_content(self):
    path = self.path
    if (len(path) > 1 and path[-1] == '/'):
      path = path[:-1]
    r = Reader(self.fs)
    return r.read_path(path)

class RM:

  def __init__(self, path, fs):
    self.path = path
    self.fs = fs
    self.filename = self._get_filename()
    self.parent_dir = self._get_parent_dir()
  
  def _get_filename(self):
    last_slash = self.path.rfind('/')
    return self.path[last_slash + 1:]
  
  def _get_parent_dir(self):
    last_slash = self.path.rfind('/')
    return self.path[:last_slash]
  
  def rm(self):
    r = Reader(self.fs)
    files, _, block = r.read_path(self.parent_dir)
    w = Writer(self.fs)
    f = self.locate_file(files)
    if not f:
      print(f'Erro: não há arquivo regular em {self.path}')
      return
    w.erase_file(block, f)
  
  def locate_file(self, files):
    f = None
    for file in files:
      if file.name == self.filename:
        f =  file
        break
    return f

class DF:

  def __init__(self, fs):
    self.fs = fs
  
  def df(self):
    r = Reader(self.fs)
    self.free, self.wasted, self.dirs, self.files = r.read_unit_stats()
    self.print_stats()
  
  def print_stats(self):
    print(f'Espaço livre: {self.free}B')
    print(f'Espaço desperdiçado: {self.wasted}B')
    print(f'Quantidade de diretórios: {self.dirs}')
    print(f'Quantidade de arquivos: {self.files}')
    
class RMDIR:

  def __init__(self, dirpath, fs):
    self.dirpath = dirpath
    self.parent_dir = self._get_parent_dir()
    self.dirname = self.get_dirname(dirpath)
    self.fs = fs

  def _get_parent_dir(self):
    last_slash = self.dirpath.rfind('/')
    return self.dirpath[:last_slash]
  
  def get_dirname(self, dirpath):
    last_slash = dirpath.rfind('/')
    return dirpath[last_slash + 1:]

  def rmdir(self):
    w = Writer(self.fs)
    # encontro os first_blocks dos blocos a serem removidos
    first_blocks_to_remove = self.get_blocks_to_remove_rec(self.dirpath)
    print("first_blocks dos arquivos a serem removidos: ", first_blocks_to_remove)
    all_blocks_to_remove = []
    # para cada first_block, encontro os blocos subjacentes e guardo tudo num único vetor all_blocks_to_remove
    for first_block in first_blocks_to_remove:
      print("Pegando os nexts do bloco", first_block, ":")
      more_blocks = w.get_all_blocks(first_block)
      all_blocks_to_remove.append(more_blocks)
    all_blocks_to_remove.sort()
    print("todos os blocos a serem removidos estão aqui: ", all_blocks_to_remove)
    first_blocks_to_remove.clear()
    all_blocks_to_remove.clear()


  """ Função recursiva que percorre todos os diretórios (a partir do diretório pai), e retorna um vetor com 
  todos os blocos ocupados pelos arquivos acessados """
  def get_blocks_to_remove_rec(self, dirpath, blocks=[]):
    r = Reader(self.fs)
    files, dirs, block = r.read_path(dirpath)
    blocks.append(block)
    dirname = self.get_dirname(dirpath)
    print("\n+++ Iniciando RMDIR do (", dirname, ") +++")
    print("(atual estado dos blocos=",blocks,')', sep='')
    print("files=", end=" ")
    if files:
      for file in files:
        print(file.name, end=" - ")
        blocks.append(file.first_block)
    else:
      print('[]', end='')
    print("\ndirs=", end=" ")
    if dirs:
      for dir in dirs:
        print(dir.name, end=" - ")
        if dirpath != '/':
          child_path = dirpath + '/' + dir.name 
        else:
          child_path = dirpath + dir.name 
        print('-> tentando entrar no ', child_path)
        blocks = self.get_blocks_to_remove_rec(child_path, blocks)
    else:
      print('[]', end='')
    print("--- Encerrando RMDIR do (", dirname, ") --- \n")

    return blocks
    
    