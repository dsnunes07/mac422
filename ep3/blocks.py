import re
from system_constants import TOTAL_BLOCKS, BLOCK_LIST_IDX
from patterns import BLOCK_START, FILE_OBJ, DIR_OBJ
from files import File, Folder

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

  def __init__(self, filename, fs):
    self.filename = filename
    self.fs = fs
  
  def read_block(self, i):
    if (i == 0):
      return self.read_root()
  
  def read_root(self):
    fp = self.open_file_at_byte(BLOCK_LIST_IDX)
    raw_root = re.sub(BLOCK_START, '', fp.readline()).rstrip()
    files = self.parse_files(raw_root)
    dirs = self.parse_dirs(raw_root)

  def parse_files(self, data):
    files = []
    for file_string in re.findall(pattern=FILE_OBJ, string=data):
      print(file_string)
    breakpoint()
    return []
  
  def parse_dirs(self, data):
    return []


  def open_file_at_byte(self, byte):
    fp = open(self.fs.filename, 'r+')
    fp.seek(byte)
    return fp
    