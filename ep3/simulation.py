import os
import re
from datetime import datetime
from fat import FAT
from bitmap import Bitmap
from blocks import BlockList, Reader, Writer
from files import File
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
      print('Nenhum arquivo montado')
      return
    print(f'desmontando {self.filename}')
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

class CP:
  
  def __init__(self, origin, destiny, fs):
    self.origin = origin
    self.destiny = destiny
    self.destiny_path = self._get_destiny_path()
    self.fs = fs
    self.content = self.read_origin_content()
  
  def read_origin_content(self):
    f = open(self.origin, 'r')
    return f.read()
  
  def _get_destiny_path(self):
    last_slash = self.destiny.rfind('/')
    return self.destiny[:last_slash]
  
  def cp(self):
    # inicia o objeto de leitura
    r = Reader(self.fs)
    # lê o conteúdo da root
    files, dirs, block = r.read_path(self.destiny_path)
    # verifica se "destino" existe
    if block == -1:
      print(f'erro: diretório {self.destiny_path} não existe!'),
      return
    self.path_block = block
    w = Writer(self.fs)
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
