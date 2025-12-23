# ChunkMemoryManager

ChunkMemoryManager é uma biblioteca de c++ que cria um alocador customizado de
memória baseado em **chunks** e oferece alternativas para **std::unique\_ptr**
e **std::make\_unique**. A biblioteca está sob a licença GNU GPLv3.

# Funcionamento

A biblioteca aloca um bloco de memória de tamanho fixo, definido pelo usuário e
o divide em pedaços iguais (chunks) de memória. Ao alocar com
cmm::make\_unique, a biblioteca procura um chunk livre e retorna a memória
alocada. Se o usuário da biblioteca define o tamanho do chunk como 64 bytes,
não importa se ele pede menos memória, os dados do usuário ocuparam todo o
espaço de 64 bytes. Se o usuário pedir para alocar um espaço maior que o chunk,
a biblioteca falha e retorna um **nullptr**.

A biblioteca é apenas um único arquivo de cabeçalho, em que o usuário pode
incluir diretamente no seu programa, para facilidade. É necessário usar
\#define para definir a implementação em um arquivo C++.

# Exemplo

Segue um exemplo de funcionamento em um arquivo:

```
#include <iostream>

#define CHUNK_MEMORY_MANAGER_IMPLEMENTATION
#include "ChunkMemoryManager.hpp"

int main(void) {
	cmm::start(1024, 1024);
	printf("Hello World!\n");

	cmm::unique_ptr<int> x;
	
	x = cmm::make_unique<int>(3);

	printf("%d\n", *x);

	return 0;
}
```

A função `cmm::start` recebe dois parâmetros: o primeiro é o número de chunks
que o usuário quer que a biblioteca aloque e o segundo é o tamanho em bytes de
cada chunk. 

# Quais são os usos

A biblioteca, sem dúvidas, desperdiça bastante memória, principalmente se o
tamanho de alocações for bastante variada. Entretanto, ela aloca de maneira
rápida, já que não são utilizados algoritmos complexos de alocação, ela apenas
retorna um chunk que está livre, independente do tamanho da alocação. Portanto,
ela é bastante útil quando se quer ter um sistema de alocação dinâmico, mas que
seja bastante rápido. Ela também é útil pois diminui o overhead de alocação no
heap, alocando um bloco de memória contínuo de uma só vez, ao invés de fazer
pequenas alocações que causam fragmentações.

