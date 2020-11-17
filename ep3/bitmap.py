from system_constants import TOTAL_BLOCKS, FAT_OFFSET, BIT_START_IDX
class Bitmap:
  
  def __init__(self, filename):
    self.filename = filename
    self.load_bitmap()

  """ cria um bitmap na unidade caso não exista e deixa ele
  carregado na memória """
  def load_bitmap(self):
    f = open(self.filename, 'r')
    f.seek(FAT_OFFSET)
    if (not 'BIT' in f.readline()):
      f.close()
      print('criando novo bitmap...')
      self.create_new_bitmap()
    self.map = self.parse_bitmap()
  
  """ escreve um bitmap vazio no sistema de arquivos recém criado """
  def create_new_bitmap(self):
    self.map = [1] * TOTAL_BLOCKS
    self.write_bitmap_to_unit()
  
  """ lê o bitmap presente na unidade, deixando-o carregado na instância """
  def parse_bitmap(self):
    f = open(self.filename, 'r')
    f.seek(BIT_START_IDX)
    str_bitmap = list(f.read())
    map = [int(pos) for pos in str_bitmap[:-1]]
    return map

  def write_bitmap_to_unit(self):
    f = open(self.filename, 'r+')
    f.seek(FAT_OFFSET)
    f.write(self.str())
    f.flush()
    f.close()

  def str(self):
    map_str = 'BIT '
    for bit in self.map:
      map_str += str(bit)
    map_str += '\n'
    return map_str