# COMANDOS

MOUNT = 'mount'
CP = 'cp'
MKDIR = 'mkdir'
RMDIR = 'rmdir'
CAT = 'cat'
TOUCH = 'touch'
RM = 'rm'
LS = 'ls'
FIND = 'find'
DF = 'df'
UMOUNT = 'umount'
SAI = 'sai'

# DADOS SOBRE O SISTEMA DE ARQUIVOS

TOTAL_BLOCKS = 24925
MAX_BLOCK_LENGTH = 4000
BASE = 16
FAT_START_IDX = 4
BIT_START_IDX = 124634
# na tabela FAT, esses valores indicam bloco vazio e último bloco:
EMPTY_BLOCK = 61166
FINAL_BLOCK = 65535
OBJ_SEPARATOR = '|'
# Número de posições que a tabela FAT ocupa
FAT_OFFSET = 124630
BLOCK_LIST_IDX = BIT_START_IDX + TOTAL_BLOCKS + 3
FIRST_BLOCK_LINE = 4
# overhead de metadados de arquivos
FILE_OVERHEAD = 64