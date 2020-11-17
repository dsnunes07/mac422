from system_constants import TOTAL_BLOCKS, FAT_START_IDX, BASE, OBJ_SEPARATOR, EMPTY_BLOCK

class FAT:
  def __init__(self, filename):
    self.filename = filename
    self.load_table()
  
  """ cria uma tabela FAT caso não exista e deixa ela
  carrega na memória """
  def load_table(self):
    f = open(self.filename, 'r')
    if (not 'FAT' in f.readline()):
      f.close()
      self.create_new_table()
    self.table = self.parse_table()
  
  """ escreve uma tabela FAT vazia no sistema de arquivos recém criado """
  def create_new_table(self):
    self.table = [EMPTY_BLOCK] * TOTAL_BLOCKS
    self.write_table_to_unit()
  
  """ carrega na memória a tabela FAT representada no arquivo """
  def parse_table(self):
    f = open(self.filename, 'r')
    table = []
    f.seek(FAT_START_IDX)
    raw_table = f.readline().split(OBJ_SEPARATOR)
    for block in raw_table[:-1]:
      table.append(int(block, BASE))
    return table
  
  """ substitui a primeira linha do arquivo, que contém a FAT obsoleta,
  pela nova FAT construída em memória """
  def write_table_to_unit(self):
    f = open(self.filename, 'r+')
    f.write(self.str())
    f.flush()
    f.close()
  
  """ gera uma representação da tabela FAT em string no formato que ela é
  representada no sistema de arquivos, em hexadecimal com separadores """
  def str(self):
    table_str = 'FAT '
    for block in self.table:
      table_str += '{:04x}|'.format(block)
    table_str += '\n'
    return table_str
