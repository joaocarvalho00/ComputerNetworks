# Redes de Computadores

### Projeto 1
#### Objetivos
* Implementar um protocolo de ligação de dados, de acordo com a
especificação a seguir descrita
* Testar o protocolo com uma aplicação simples de transferência de
ficheiros, igualmente especificada

<p>
  <br>
</p>


Aplicações                 |  Chamada
---------------------------|:-------------------------:|
 Emissor                   |  <p>{Nome da aplicação} {nome da porta de série} {modo de abertura (w)} {nome do ficheiro a enviar} {tempo de timeout} {número de tentativas máximas de reenvio} <br><br> **Ex:** app /dev/ttyS0 w pinguim.gif 3 3 </p>
 Recetor                   | <p>{Nome da aplicação} {nome da porta de série (ex: /dev/ttyS0)} {modo de abertura (r)} {tempo de timeout} {número de tentativas máximas de reenvio} <br><br> **Ex:** app /dev/ttyS0 r 3 3<p>


***

### Projeto 2
#### Objetivos
* Desenvolver uma aplicação de download, segundo o portocolo FTP
* Configuração de uma rede de computadores

<p>
  <br>
</p>


Aplicação                  |  Chamada
---------------------------|:-------------------------:|
Download                   |  <p>download ftp://{user}:{password}@{host}/{url-path} <br><br> **Ex:** download ftp://demo:password@test.rebex.net/pub/example/mail-editor.pt </p>


***

### Developers

* [Carlos Freitas](https://github.com/CarlosFr97)
* [David Falcão](https://github.com/davidrsfalcao)
* [Luís Martins](https://github.com/luisnmartins)
