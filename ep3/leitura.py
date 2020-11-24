f = open('unidade', 'r')
f.seek(121509 + 24300 + 3)
n = int(input('Qual bloco?'))

block = -1
conteudo = ''

while (block != n):
  conteudo = f.readline()
  block+=1

print('conteudo da linha:')
print(conteudo)