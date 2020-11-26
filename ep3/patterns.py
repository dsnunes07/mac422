# Captura o começo de um bloco
BLOCK_START = r'^[a-f0-9]+[ ]'
# Captura a string representando arquivo
FILE_OBJ = r'([\^][\w.]{1,16}&[a-f0-9A-F]{7}(?:(?:[&][a-fA-F0-9]+){4}))'
# Captura a string representando diretório
DIR_OBJ = r'([\%][\w.]{1,16}(?:(?:[&][a-fA-F0-9]+){4}))'

ENTRY_BY_NAME = r'([\^|\%]\b(name)\b[&]{0,1}[a-f0-9A-F]{0,7}(?:(?:[&][a-f0-9]+){4})\|)'
