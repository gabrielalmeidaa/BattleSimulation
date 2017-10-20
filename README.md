#Projeto de Programação Concorrente
Aluno: Gabriel Almeida Campos.	
Matrícula: 15/0009887
##Descrição
O projeto é uma simulação de batalha de 2 times de arqueiros. Cada time possui **N** arqueiros, uma equipe auxiliar de **M** pessoas, um Armazém e uma Pilha de Materiais. Os Arqueiros permancem em um campo, lutando entre si, enquanto os assistentes realizam outras ações. A simulação acaba quando todos os arqueiros de um time forem derrotados.

##Regras
Existe um conjunto de regras para cada entidade do projeto.

###Arqueiros
Cada arqueiro possui um aljave de flechas com determinada quantidade e um número de vida. Realizam as seguintes ações:
* Atirar flechas
	* Quando estiver sem flechas, pedem assistência e se escondem _(sleep)_.

###Assistentes
Os assistentes podem realizar entre as seguintes tarefas, determinadas aleatóriamente:
* Produzir flechas.
	* Para produzir flechas, é necessário materiais. Caso não possua, solicita ajuda à outro assistente para buscar materiais.
* Buscar materiais.
* Invadir o Armazém inimigo para roubar flechas.
	* Se existir um inimigo em _Idle_ , o assistente deve esperar até o armazém estar vazio.
	* Se não existirem flechas no armazém, o assistente deve ir embora.
* Ficar em _Idle (sleep)_.
	* Um assistente em _Idle_, ao ser solicitado ajuda por um arqueiro, leva flechas à este arqueiro. Após, realiza outra ação definida aleatóriamente.
	* Um assistente em _Idle_, ao ser solicitado ajuda por um assistente, sai para buscar materiais para fabricação de flechas. Após, realiza outra ação definida aleatóriamente.
