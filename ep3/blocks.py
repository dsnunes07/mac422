import re
from system_constants import TOTAL_BLOCKS, BLOCK_LIST_IDX, FINAL_BLOCK, EMPTY_BLOCK, MAX_BLOCK_LENGTH, BIT_START_IDX
from patterns import BLOCK_START, FILE_OBJ, DIR_OBJ, ENTRY_BY_NAME
from files import File, Directory
import fileinput
from math import ceil

class BlockList:
  
  def __init__(self, filename, new_one):
    self.filename = filename
    # cria a lista de blocos e a root em caso de novo arquivo
    if (new_one):
      self.create_block_list()
  
  def create_block_list(self):
    f = open(self.filename, 'r+')
    f.seek(BLOCK_LIST_IDX)
    for i in range(TOTAL_BLOCKS):
      f.write('{:04x} \n'.format(i))

class Reader:

  def __init__(self, fs):
    self.fs = fs
  
  def read_block(self, i):
    files = []
    dirs = []
    while (i != FINAL_BLOCK and i != EMPTY_BLOCK):
      content = self.raw_block_content(i)
      files.extend(self.parse_files(content))
      dirs.extend(self.parse_dirs(content))
      i = self.fs.fat.table[i]
    return files, dirs    

  def parse_files(self, data):
    files = []
    for file_string in re.findall(pattern=FILE_OBJ, string=data):
      file_splitted = file_string.split('&')
      name = file_splitted[0].replace('^', '')
      size = int(file_splitted[1], 16)
      created_at = int(file_splitted[2])
      accessed_at = int(file_splitted[3])
      modified_at = int(file_splitted[4])
      first_block = int(file_splitted[5], 16)
      file = File(name, size, created_at, accessed_at, modified_at, first_block)
      files.append(file)
    return files
  
  def parse_dirs(self, data):
    dirs = []
    for dir_string in re.findall(pattern=DIR_OBJ, string=data):
      dir_splitted = dir_string.split('&')
      name = dir_splitted[0].replace('%', '')
      created_at = int(dir_splitted[1])
      accessed_at = int(dir_splitted[2])
      modified_at = int(dir_splitted[3])
      first_block = int(dir_splitted[4], 16)
      directory = Directory(name, created_at, accessed_at, modified_at, first_block)
      dirs.append(directory)
    return dirs
  
  def raw_block_content(self, block_number):
    f = open(self.fs.filename, 'r')
    f.seek(BLOCK_LIST_IDX)
    block = -1
    raw_content = ''
    while (block != block_number):
      raw_content = f.readline()
      block += 1
    f.close()
    return raw_content
  
  """ Recebe uma string com o caminho de um diretório e retorna uma
  tupla contendo as listas files e dirs, com o conteúdo desse diretório.
  Se o caminho não existir, retorna listas vazias e -1 como bloco """
  def read_path(self, path):
    # ler tudo da root
    files, dirs = self.read_block(0)
    path_split = path.split('/')
    path_block = -1
    if (path == '/' or len(path_split) == 1):
      path_block = 0
      return files, dirs, path_block
    # buscar os diretórios
    for p in path_split[1:]:
      dir_found = False
      for d in dirs:
        if (d.name == p):
          files, dirs = self.read_block(d.first_block)
          dir_found = True
          path_block = d.first_block
          break
      if (not dir_found):
        return [], [], path_block
    return files, dirs, path_block
  
  def print_file_content(self, file):
    file_address = '{:04x}'.format(file.first_block)
    block = file.first_block
    f = open(self.fs.filename, 'r')
    f.seek(BLOCK_LIST_IDX)
    while (block != FINAL_BLOCK):  
      line = f.readline()
      if line[0:4] == file_address:
        print(line[5:].rstrip(), end='')
        f.seek(BLOCK_LIST_IDX)
        block = self.fs.fat.table[block]
    f.close()
  
  """ Extrai estatísticas (espaço livre, desperdiçado e quantidade de arquivos e
  diretórios) da unidade """
  def read_unit_stats(self):
    f = open(self.fs.filename, 'r')
    f.seek(BLOCK_LIST_IDX)
    line = f.readline().rstrip()
    free_space = 0
    wasted_space = 0
    n_dirs = 0
    n_files = 0
    while (line != ''):
      address_int = int(line[0:4], 16)
      if self.fs.bitmap.map[address_int] == 1:
        free_space += MAX_BLOCK_LENGTH
      else:
        wasted_space += (MAX_BLOCK_LENGTH - len(line[5:]))
        n_dirs += len(re.findall(DIR_OBJ, line))
        n_files += len(re.findall(FILE_OBJ, line))
      line = f.readline().rstrip()
    return free_space, wasted_space, n_dirs, n_files
  

  


class Writer:

  def __init__(self, fs):
    self.fs = fs

  """ checa se a entry (no format *name&timestamp&timestamp...) pode ser escrita no bloco - ou seja, se o bloco não
  vai ultrapassar o tamanho de 4096 bits ao escrever a entrada no diretório """
  def check_entry_fits_block(self, block_number, entry):
    r = Reader(self.fs)
    raw_content = r.raw_block_content(block_number)
    return (len(raw_content) + len(entry)) < 4096
  
  """ Dado o primeiro bloco, retorna o último bloco do arquivo """
  def get_last_block(self, parent_block):
    end = parent_block
    while (end != FINAL_BLOCK):
      last_block = end
      end = self.fs.fat.table[last_block]
    return last_block

  """ Dado o primeiro bloco, retorna um vetor com todos os blocos do arquivo"""
  def get_all_blocks(self, parent_block):
    all_blocks = []
    end = parent_block
    while (end != FINAL_BLOCK):
      last_block = end
      all_blocks.append(last_block)
      end = self.fs.fat.table[last_block]
    print("Achei todos estes: ", all_blocks)
    return all_blocks

  """ Recebe o bloco dir do diretório que contém o arquivo, recebe a string com
  a entrada na tabela de diretórios e recebe o objeto do arquivo a ser escrito,
  que contém seu conteúdo. Essa função também atualiza a FAT e o bitmap na memória """
  def write_file(self, dir, entry, file):
    last_block = self.update_dir_last_block(dir, entry)
    dir_address = '{:04x}'.format(last_block)
    current_block = file.first_block
    file_address = '{:04x}'.format(current_block)
    for line in fileinput.FileInput(self.fs.filename, inplace=1):
      # escrever a entrada do diretório
      if line[0:4] == dir_address:
        line = line.replace('\n', '')
        line += f'{entry}\n'
      elif line[0:4] == file_address:
        if (current_block != FINAL_BLOCK):
          current_content = file.content[:4096]
          line = f'{line[0:5]}{current_content}\n'
          file.content = file.content[4096:]
          self.fs.bitmap.map[current_block] = 0
          if not file.content:
            next_block = FINAL_BLOCK
          else:
            next_block = self.fs.nearest_empty_block(current_block)
          self.fs.fat.table[current_block] = next_block
          current_block = next_block
          file_address = '{:04x}'.format(current_block)
        else:
          line = f'{line[:5]}{file.content}\n'
      print(line, end='')
  
  """ Recebe o bloco onde a entrada está e o nome do arquivo/diretório a ser
  apagado. Exclui a entrada da unidade e todo o seu conteúdo associado """
  def erase_file(self, block, file):
    # caso o arquivo esteja distribuído em vários blocos, apaga todos eles
    blocks = []
    file_last_block = file.first_block
    while (file_last_block != FINAL_BLOCK):
      blocks.append(file_last_block)
      file_last_block = self.fs.fat.table[file_last_block]
    # ordena os blocos para remover na ordem da varredura
    blocks.sort()
    block_i = 0
    hex_address = '{:04x}'.format(block)
    content_address = '{:04x}'.format(blocks[block_i])
    for line in fileinput.FileInput(self.fs.filename, inplace=1):
      if line[:4] == hex_address:
        line = re.sub(ENTRY_BY_NAME.replace('(name)', file.name), '', line)
      elif line[:4] == content_address:
        line = f'{line[:4]} \n'
        self.fs.fat.table[blocks[block_i]] = EMPTY_BLOCK
        self.fs.bitmap.map[blocks[block_i]] = 1
        block_i+=1
        if block_i < len(blocks):
          content_address = '{:04x}'.format(blocks[block_i])
      print(line, end='')
  
  def update_entry(self, file, block, new_entry):
    block_address = '{:04x}'.format(block)
    for line in fileinput.FileInput(self.fs.filename, inplace=1):
      if line[:4] == block_address:
        pattern = ENTRY_BY_NAME.replace('(name)', file.name)
        line = re.sub(pattern, new_entry, line)
      print(line, end='')

  def write_fat(self):
    self.fs.fat.write_table_to_unit()
  
  def write_bitmap(self):
    self.fs.bitmap.write_bitmap_to_unit()

  """ Recebe o first_block do diretório pai onde deve ocorrer a escrita e a 
  entrada do novo diretório (no formato %dirname&timestamp&timestamp&timestamp&first_block) a ser criado """
  def write_directory(self, parent_block, entry):
    # Encontra o último bloco do diretório
    last_block = self.update_dir_last_block(parent_block, entry)
    dir_address = '{:04x}'.format(last_block)
    # Escreve a entrada no sistema de arquivos.
    for line in fileinput.FileInput(self.fs.filename, inplace=1):
      if line[0:4] == dir_address:
          line = line.replace('\n', '')
          line += f'{entry}\n'
      print(line, end='')

  def update_dir_last_block(self, parent_block, entry):
     # pega o último bloco do diretório pai (em teoria, é onde é possível escrever as entradas)
    last_block = self.get_last_block(parent_block)
    # se não cabe mais entradas nesse bloco, pega o próximo bloco vazio, atualiza a tabela FAT, o BITMAP
    # e o endereço onde a entrada deve ser escrita
    if not self.check_entry_fits_block(last_block, entry):
      next_block = self.fs.nearest_empty_block(last_block)
      self.fs.bitmap.map[next_block] = 0
      self.fs.fat.table[last_block] = next_block
      self.fs.fat.table[next_block] = FINAL_BLOCK
      last_block = next_block
    return last_block 