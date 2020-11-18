from system_constants import FILE_OVERHEAD
from datetime import datetime

""" superclasse File, recebe um caminho e permite que o conteúdo do objeto
# seja definido """
class File:

  def __init__(self, path, fs):
    self.path = path
    self.fs = fs
  
  def set_content(self, content):
    self.content = content
    self.size_bytes = FILE_OVERHEAD + len(content.encode('utf-8'))

""" Classe derivada de File, Inner, para representar um arquivo interno ao sistema de arquivos,
isto é, que está ou será escrito na unidade simulada """
class Inner(File):

  def __init__(self, path, fs):
    File.__init__(self, path, fs)

  def exists(self):
    return False
  
  def write_to_unit(self):
    self.fs.create_or_update_entry(self.path, self.content)
    pass

""" Classe derivada de File, Outer, para representar um arquivo externo ao sistema de arquivos, recebido no primeiro argumento
do comando cp (origem) """

class Outer(File):

  def __init__(self, path):
    File.__init__(self, path, None)
    self.read_external_file_content()
  
  def read_external_file_content(self):
    f = open(self.path, 'r')
    content = f.read()
    self.set_content(content)

""" Classe que faz o comando cp """
class CopyFiles:

  def __init__(self, origin_path, destiny_path, fs):
    self.origin = Outer(origin_path)
    self.destiny = Inner(destiny_path, fs)
    self.fs = fs
  
  def copy_to_unit(self):
    self.destiny.set_content(self.origin.content)
    self.destiny.write_to_unit()

""" Abstração de um diretório, cada instância contém também uma lista de diretórios e uma lista de arquivos,
representando o conteúdo """
class Folder:

  def __init__(self, name, start_block, created_at, modified_at, accessed_at):
    self.name = name
    self.created_at = created_at
    self.modified_at = modified_at
    self.accessed_at = accessed_at
    self.start_block = start_block
    self.files = []
    self.folders = []
  
  def append_file(self, file):
    self.files.append(file)
  
  def append_folder(self, folder):
    self.folders.append(folder)

# retorna o momento atual em timestamp unix
def now():
  return int(datetime.now().timestamp())