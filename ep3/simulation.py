import os
import re
from datetime import datetime
from fat import FAT
from bitmap import Bitmap
from blocks import BlockList, Reader, Writer
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
      print(f'Erro: {self.destiny_path} não existe')
      return
    breakpoint()
    w = Writer(self.fs)
    print('Copiando')
