import re
from system_constants import TOTAL_BLOCKS, BLOCK_LIST_IDX, FINAL_BLOCK, EMPTY_BLOCK
from patterns import BLOCK_START, FILE_OBJ, DIR_OBJ
from files import File, Directory
import fileinput

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
      created_at = file_splitted[2]
      modified_at = file_splitted[3]
      accessed_at = file_splitted[4]
      first_block = int(file_splitted[5], 16)
      file = File(name, size, created_at, modified_at, accessed_at, first_block)
      files.append(file)
    return files
  
  def parse_dirs(self, data):
    dirs = []
    for dir_string in re.findall(pattern=DIR_OBJ, string=data):
      dir_splitted = dir_string.split('&')
      name = dir_splitted[0].replace('%', '')
      created_at = dir_splitted[1]
      modified_at = dir_splitted[2]
      accessed_at = dir_splitted[3]
      first_block = int(dir_splitted[4], 16)
      directory = Directory(name, created_at, modified_at, accessed_at, first_block)
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
  Se o caminho não existir, retorna None """
  def read_path(self, path):
    # ler tudo da root
    files, dirs = self.read_block(0)
    path_split = path.split('/')
    path_block = -1
    if (len(path_split) == 1):
      path_block = 0
      return dirs, files, path_block
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
        return None, None, path_block
    return dirs, files, path_block

class Writer:

  def __init__(self, fs):
    self.fs = fs
  
  """ Escreve a string content no bloco 'block'. Qualquer conteúdo existente em block
  será apagado antes da escrita """
  def write_to_block(self, block, content):
    f = open(self.fs.filename)
  
  """ Acrescenta a string content ao conteúdo já existente no bloco "block" """
  def append_to_block(self, block, content):
    r = Reader(self.fs)
    block_old_content = r.raw_block_content(block)
    for line in fileinput.FileInput(self.fs.filename, inplace=1):
      if line[0:4] == '{:04x}'.format(block):
        line = line.replace('\n', '')
        line += f'{content}\n'
      print(line, end='')
  
  """ Recebe o bloco dir do diretório que contém o arquivo, recebe a string com
  a entrada na tabela de diretórios e recebe  o objeto do arquivo a ser escrito,
  que contém seu conteúdo """
  def write_file(self, dir, entry, file):
    dir_address = '{:04x}'.format(dir)
    file_address = '{:04x}'.format(file.first_block)
    for line in fileinput.FileInput(self.fs.filename, inplace=1):
      # escrever a entrada do diretório
      if line[0:4] == dir_address:
        line = line.replace('\n', '')
        line += f'{entry}\n'
      elif line[0:4] == file_address:
        line = f'{line[0:5]}{file.content}\n'
      print(line, end='')

