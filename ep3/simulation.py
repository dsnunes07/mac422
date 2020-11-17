import os
from datetime import datetime
from fat import FAT
from bitmap import Bitmap

class FileSystem:
  """ valores acessíveis:
  filename: nome do arquivo da unidade
  fat: instância da FAT na memória
  bitmap: instância do BITMAP na memória """
  def mount(self, filename):
    self.filename = filename
    self.load_filesystem()

  def umount(self):
    print(f'desmontando {self.filename}')
  
  # carrega a unidade ou cria uma nova caso ela não exista em 'filename'
  def load_filesystem(self):
    if (not os.path.isfile(self.filename)):
      self.create_filesystem()
    self.fat = FAT(self.filename)
    self.bitmap = Bitmap(self.filename)
    breakpoint()

  # inicializa a unidade totalmente vazia
  def create_filesystem(self):
    if ('/' in self.filename):
      os.makedirs(os.path.dirname(self.filename), exist_ok=True)
    fp = open(self.filename, 'w')
    fp.close()

class Folder:
  
  def create_new(self, name):
    self.created_at = datetime.now()
    self.updated_at = datetime.now()
    self.accessed_at = datetime.now()
    self.in_folder = []
