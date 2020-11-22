# Captura o começo de um bloco
BLOCK_START = r'^[a-f0-9]+[ ]'
# Captura a string representando arquivo
FILE_OBJ = r'([\^][\w.]{1,16}&[a-f0-9A-F]{7}(?:(?:[&][0-9]+){4}))'
# Captura a string representando diretório
DIR_OBJ = r'([\%][\w.]{1,16}(?:(?:[&][0-9]+){4}))'