from cprompt import Command
from simulation import FileSystem, file_system

def main():
  user_input = ''
  file_system = FileSystem()  
  while user_input != 'sai':
    user_input = input('[ep3]: ')
    command = Command(user_input, file_system)
    command.execute()

main()