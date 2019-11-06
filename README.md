# Interpretador Lua
Interpretador de Lua v1.0 básico para a matéria de ITC, com foco em automatos.

## Compilar:
Para compilar o código, basta utilizar o CMake:

### Linux:

* Criar uma nova pasta no diretório raiz e se mover para ela: `mkdir build && cd build`
* Gerar os arquivos para a compilação do código: `cmake ..`
* Compilar o código: `make`

## Rodando o interpretador:
Para rodar o interpretador, é necessário (*por enquanto*) ter um arquivo com o código lua. Um código-exemplo vem junto com o programa, o **test.lua**.

* Basta executar o binário gerado pela compilação (*luaconsole*), passando o arquivo lua como parametro. E.g.: `./luaconsole ../test.lua`
