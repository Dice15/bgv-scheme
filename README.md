# bgv-scheme
 Leveled Fully Homomorphic Encryption without Bootstrapping


# test
### Context

- poly modulus degree: 4
- plain modulus value: 32713
- level: 3

- param
  - level: 3, next level: 2, q: 1148844456385434517
  - level: 2, next level: 1, q: 35056740910727
  - level: 1, next level: 0, q: 1070467523
  - level: 0, next level: 2, q: 32717

### KeyGenerator

- secret key
  
  - l = 3
    - 1 1 1 1
    - 2 1 6 7

  - l = 2
    - 1 1 1 1
    - 4 1 1 3

  - l = 1
    - 1 1 1 1
    - 2 1 3 1

  - l = 0
    - 1 1 1 1
    - 0 0 3 1


- public key

  - l = 3
    - 65438 261707 98151 65447
    - -6 -3 -2 -3

  - l = 2
    - 65434 163566 98140 12
    - -2 -1 -1 -4

  - l = 1
    - 98143 163567 130858 65430
    - -2 -2 -2 -4

  - l = 0
    - 32705 32697 4 32712
    - -6 -1 -4 -7

### BGV Encoder

- encode and decode
  - 1 -15
  - 2 -5
  - 3 5
  - 4 15

### BGV Encryptor

- encrypt
  - 196315 -18
  - 817836 65417
  - 392595 32707
  - 229058 32704

### BGV Decryptor

- decrypt
  - 1 -15
  - 2 -5
  - 3 5
  - 4 15

### BGV Modulus Switching

- l = 3 -> 2
- q: 1148844456385434517 -> 35056740910727
  - 1 -15
  - 2 -5
  - 3 5
  - 4 15

- l = 2 -> 1
- q: 35056740910727 -> 1070467523
  - 1 -15
  - 2 -5
  - 3 5
  - 4 15

- l = 1 -> 0
- q: 1070467523 -> 32717
  - 1 -15
  - 2 -5
  - 3 5
  - 4 15
