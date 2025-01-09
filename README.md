# bgv-scheme
 Leveled Fully Homomorphic Encryption without Bootstrapping
 
# test

임시 사진 (이후에 삭제할 예정)

![image](https://github.com/user-attachments/assets/7671abcc-a347-4ec4-878c-2e44fce77e3e)

![image](https://github.com/user-attachments/assets/8c93550b-551b-4bd8-890b-97aa4de5aaa6)

### Context

- poly modulus degree: 4
- plain modulus value: 1009
- depth: 3

- param
  - level: 3, next level: 2, q: 1048583
  - level: 2, next level: 1, q: 1048589
  - level: 1, next level: 0, q: 1048601
  - level: 0, next level: -1, q: 1048609

### KeyGenerator

- secret key
  
  - l = 3, dimension = 2 x 1
    - $$t^0$$: [ [ 1 ] ]
    - $$t^1$$: [ [ 2 ] ]

  - l = 2, dimension = 2 x 1
    - $$t^0$$: [ [ 1 ] ]
    - $$t^1$$: [ [ 1 ] ]

  - l = 1, dimension = 2 x 1
    - $$t^0$$: [ [ 1 ] ]
    - $$t^1$$: [ [ 1 ] ]

  - l = 0, dimension = 2 x 1
    - $$t^0$$: [ [ 1 ] ]
    - $$t^1$$: [ [ 6 ] ]


- public key

  - l = 3, dimension = 1 x 2
    - [ [ 199544 913648 334941 238096 ] [ 426033 594786 356821 931553 ] ]

  - l = 2, dimension = 1 x 2
    - [ [ 129497 222336 588065 194940 ] [ 921110 826253 461533 853649 ] ]

  - l = 1, dimension = 1 x 2
    - [ [ 93001 97948 585693 77190 ] [ 962663 952671 463917 971411 ] ]

  - l = 0, dimension = 1 x 2
    - [ [ 102457 371917 2278 301520 ] [ 333301 288391 293 824092 ] ]

### BGV Encoder

- encode and decode
  - v1 = [ 1 2 3 4 ]
  - v2 = [ -15 -5 5 15 ]

### BGV Encryptor

- encrypt
  - c1 = [ 353271 111103 252469 620111 ]
  - c2 = [ 887083 1019771 402599 230382 ]

### BGV Decryptor

- decrypt
  - v1 = [ 1 2 3 4 ]
  - v2 = [ -15 -5 5 15 ]
