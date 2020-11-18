import os
import re
from datetime import datetime
from fat import FAT
from bitmap import Bitmap
from blocks import BlockList, Reader
from system_constants import BLOCK_LIST_IDX
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

  def create_or_update_entry(self, path, content):
    self.search_entry(path)
  
  def search_entry(self, path):
    reader = Reader(path, self)
    reader.read_block(0)
