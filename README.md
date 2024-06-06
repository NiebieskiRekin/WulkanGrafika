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
    - &larr; - obrót kamery w lewą stronę
    - &rarr; - obrót kamery w prawą stronę
    - &uarr; - obrót kamery do góry
    - &darr; - obrót kamery w dół

    Możliwy jest obrót dookoła wulkanu (360° lewo-prawo) oraz obrót ograniczony od 2.5° do ok. 60° góra-dół.

---

# Wykorzystane elementy zewnętrzne:

## Wykorzystane tekstury:
  - textury wulkanu oraz terenu:

    ColorMap texture został przerobiony w programie Krita

    [https://www.turbosquid.com/3d-models/volcano-terrain-2202730](https://www.turbosquid.com/3d-models/volcano-terrain-2202730)

  - textury lawy oraz cząsteczek, które powstają podczas wybuchu:

    textura metal.png (dostarczona wraz z szablonem do laboratorium 11), której kolory zostały przerobione w programie Krita
    (przeniesienie koloru z całej czerwonej tekstury)

  - textury drzew:

    [https://www.turbosquid.com/3d-models/low-poly-winter-tree-3d-model-2007551](https://www.turbosquid.com/3d-models/low-poly-winter-tree-3d-model-2007551)

  - textury dinozaura:

    [https://www.turbosquid.com/3d-models/tyrannosaurus-rex-animated-rigged-3d-2036033](https://www.turbosquid.com/3d-models/tyrannosaurus-rex-animated-rigged-3d-2036033)

  - Wszystkie textury specular powstały poprzez przerobienie textur bazowych w programie Krita (selekcja wybranych nierefleksyjnych obszarów wg. koloru, zamalowanie na czarno, odwrócenie zaznaczenia, losowy szum, mapa wypukłości Phonga z dwoma białymi źródłami światła)


## Wykorzystane modele:
  - model wulkanu

    [https://www.cgtrader.com/free-3d-models/exterior/landscape/low-poly-volcano-63a93e9e-d61e-4824-923b-00c6c02303e4](https://www.cgtrader.com/free-3d-models/exterior/landscape/low-poly-volcano-63a93e9e-d61e-4824-923b-00c6c02303e4)

    przerobiony bazowy model w programie blender, aby uzyskać większą szczegółowość 
    (wykorzystano głównie modyfikator subdivision oraz wprowadzono delikatne poprawki wizualne)

  - model terenu

    stworzony samodzielnie w programie blender

  - model lawy

    stworzony samodzielnie w programie blender, na podstawie modelu wulkanu

  - model cząstek

    stworzony samodzielnie w programie blender

  - model drzew

    [https://www.turbosquid.com/3d-models/low-poly-winter-tree-3d-model-2007551](https://www.turbosquid.com/3d-models/low-poly-winter-tree-3d-model-2007551)

  - model dinozaura

    [https://www.turbosquid.com/3d-models/tyrannosaurus-rex-animated-rigged-3d-2036033](https://www.turbosquid.com/3d-models/tyrannosaurus-rex-animated-rigged-3d-2036033)

