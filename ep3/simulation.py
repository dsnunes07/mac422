import os
from datetime import datetime

class FileSystem:
  def mount(self, filename):
    self.fsystem = filename
    if (os.path.isfile(filename)):
      self.load_filesystem(filename)
    else:
      self.create_filesystem(filename)

  def umount(self):
    print(f'desmontando {self.fsystem}')
  
  def load_filesystem(self, filename):
    f = open(filename)
    print(f.read())
    pass

  def create_filesystem(self, filename):
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    f = open(filename, 'w')
    f.write('brand new filesystem')
    f.close()

class Folder:
  
  def create_new(self, name):
    self.created_at = datetime.now()
    self.updated_at = datetime.now()
    self.accessed_at = datetime.now()
    self.in_folder = []
