# DA Project 2: Compiler Register Allocation Algorithms

Este projeto foi desenvolvido no âmbito da unidade curricular de Desenho de Algoritmos (DA). O objetivo é implementar uma ferramenta de alocação de registos para o back-end de um compilador, utilizando algoritmos e heurísticas de coloração de grafos.

## ⚙️ Pré-requisitos

Para compilar este projeto, necessitas de ter instalado no teu sistema:
* **CMake** (versão 3.10 ou superior)
* Um compilador de **C++** que suporte a norma C++17

## 🛠️ Como Compilar (Build)

Recomenda-se a compilação numa pasta separada (`build`) para manter o código-fonte limpo. Executa os seguintes comandos dentro da pasta `code`:

```bash
mkdir build
cd build
cmake ..
cmake --build .
./myProg