from system_constants import *

class Command:

  def __init__(self, command, file_system):
    self.command = command
    self.file_system = file_system

  def str(self):
    return self.command
  
  def execute(self):
    input_split = self.command.split(' ')
    command = input_split[0]
    if (command == MOUNT):
      self.file_system.mount(input_split[1])
    elif (command == CP):
      print('é o cp!')
    elif (command == MKDIR):
      print('é o mkdir')
    elif (command == RMDIR):
      print('é o rmdir!')
    elif (command == CAT):
      print('é o cat!')
    elif (command == TOUCH):
      print('é o touch!')
    elif (command == RM):
      print('é o rm!')
    elif (command == LS):
      print('é o ls!')
    elif (command == FIND):
      print('é o find!')
    elif (command == DF):
      print('é o df!')
    elif (command == UMOUNT):
      self.file_system.umount()
    elif (command == SAI):
      pass
    else:
      print(f'{command}: comando não encontrado.')
