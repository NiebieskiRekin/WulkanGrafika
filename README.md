---
header-includes:
 - \usepackage{fvextra}
 - \DefineVerbatimEnvironment{Highlighting}{Verbatim}{breaklines,commandchars=\\\{\}}
---

Instrukcja do projektu z Grafiki i Wizualizacji
========================
1. Sklonuj projekt `git clone https://github.com/NiebieskiRekin/WulkanGrafika.git` lub pobierz archiwum
2. Zainstaluj wymagane zależności:    
    - Fedora:  
    
       ```bash
       sudo dnf install kernel-devel gcc-c++ make glew-devel glew glfw-devel glfw assimp assimp-devel glm
       ``` 
    - Ubuntu etc. 
    
       ```bash
       sudo apt update && sudo apt upgrade && sudo apt install linux-headers-generic gcc make glew-utils libglew-dev libglfw3 libglfw3-dev libglfw3 libassimp-dev build-essential libglm-dev
       ```
3. Jeśli pobrane zostało archiwum - rozpakuj pobrany szkielet programu do preferowanej lokalizacji np. `~/ProjektGrafika`  
    - Otwórz File manager np. `nautilus`, prawy przycisk myszy na archiwum, rozpakuj
    - Alternatywnie: 
    
      ```bash
      mkdir -p ~/ProjektGrafika && cd ~/ProjektGrafika
      ```
      
      ```bash
      tar -xf ~/Pobrane/projekt.zip
      ```
4. Otwórz rozpakowany katalog `cd ~/ProjektGrafika/projekt`
5. Kompilacja i uruchomienie (w `~/ProjektGrafika/projekt`):
    - `cd src && make && ./main_file`
6. Sterowanie kamerą:
    - &larr; - obrót w lewo
    - &rarr; - obrót w prawo
    - &uarr; - obrót w górę
    - &darr; - obrót w dół
