import system_constants as c
from simulation import CP
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
      cp.cp()
    elif (command == c.MKDIR):
      print('é o mkdir')
    elif (command == c.RMDIR):
      print('é o rmdir!')
    elif (command == c.CAT):
      print('é o cat!')
    elif (command == c.TOUCH):
      print('é o touch!')
    elif (command == c.RM):
      print('é o rm!')
    elif (command == c.LS):
      print('é o ls!')
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
