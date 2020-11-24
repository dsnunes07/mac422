from system_constants import FILE_OVERHEAD
from datetime import datetime

class File:

  def __init__(self, name='', size=0, created_at=0, modified_at=0, accessed_at=0, first_block=0):
    self.name = name
    self.size = size
    self.created_at = created_at
    self.modified_at = modified_at
    self.accessed_at = accessed_at
    self.first_block = first_block
  
  def set_content(self, content):
    self.size = len(content.encode('utf-8'))
    self.content = content

class Directory:

  def __init__(self, name='', created_at=0, accessed_at=0, modified_at=0, first_block=0):
    self.name = name
    self.created_at = created_at
    self.modified_at = modified_at
    self.accessed_at = accessed_at
    self.first_block = first_block
    self.files = []
  
  def add_file(self, file):
    self.files.append(file)
