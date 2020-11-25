import system_constants as c
from simulation import CP, Touch, MKDIR, CAT, LS, RM
import time

class Command:

  def __init__(self, command, file_system):
    self.command = command
    self.file_system = file_system

  def str(self):
    return self.command

  def execute(self):
    input_split = self.command.split(' ')
    command = input_split[0]
    if (command == c.MOUNT):
      self.file_system.mount(input_split[1])
    elif (command == c.CP):
      origin = input_split[1]
      destiny = input_split[2]
      cp = CP(origin, destiny, self.file_system)
      start = time.time()
      cp.cp()
      end = time.time()
      print(f'Tempo decorrido: {end - start}s')
    elif (command == c.MKDIR):
      destiny = input_split[1]
      mkdir = MKDIR(destiny, self.file_system)
      mkdir.mkdir()
    elif (command == c.RMDIR):
      print('é o rmdir!')
    elif (command == c.CAT):
      cat = CAT(input_split[1], self.file_system)
      cat.cat()
    elif (command == c.TOUCH):
      file = input_split[1]
      touch = Touch(file, self.file_system)
      touch.touch()
    elif (command == c.RM):
      path = input_split[1]
      rm = RM(path, self.file_system)
      start = time.time()
      rm.rm()
      end = time.time()
      print(f'Tempo decorrido: {end-start}s')
    elif (command == c.LS):
      path = input_split[1]
      ls = LS(path, self.file_system)
      ls.ls()
    elif (command == c.FIND):
      print('é o find!')
    elif (command == c.DF):
      print('é o df!')
    elif (command == c.UMOUNT):
      self.file_system.umount()
    elif (command == c.SAI):
      pass
    else:
      print(f'{command}: comando não encontrado.')
