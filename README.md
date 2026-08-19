# Moon Render

This is a renderer written based on Qt+Opengl

## Plans to do

### Rendering

- Performance Optimization
  - [ ] lod
  - [ ] grid mesh 
- rendering effect
  - [ ] 2D Gui for interactive on screen
  - [ ] sketcher contraint draw
  - [ ] Global lighting
  - [x] shadow on reflect plane
  - [ ] ssao improve
  - [ ] transparent imporve
- PathTrace
  - [ ] CPU
  - [ ] GPU on CUDA
  - [x] GPU on fragment



- ```
  Performance Optimization
  ```

## Viewer

### OCC geometric modeling

------

![image-20260521203930108](README.assets/image-20260521203930108.png)

![image-20260614223107815](README.assets/image-20260614223107815.png)

![](README.assets/image-20260530091405678.png)

![image-20260607084702171](README.assets/image-20260607084702171.png)

![image-20260607085050874](README.assets/image-20260607085050874.png)

![image-20260611002833901](README.assets/image-20260611002833901.png)

### pbr and pathtrace

------

![image-20260315105534305](README.assets/image-20260315105534305.png)

![image-20260315105623450](README.assets/image-20260315105623450.png)

![image-20260315105725010](README.assets/image-20260315105725010.png)

![path](README.assets/path.png)

![image-20260428220406581](README.assets/image-20260428220406581.png)

![image-20260102211418374](README.assets/image-20260102211418374.png)

![image-20260316220501762](README.assets/image-20260316220501762.png)

![image-20260108201518310](README.assets/image-20260108201518310.png)



![image-20250929212947417](README.assets/image-20250929212947417.png)

### ssao

![image-20260402203337507](README.assets/image-20260402203337507.png)

### transparent

------

depth peel

![image-20260325225058918](README.assets/image-20260325225058918.png)

### clip

![image-20260401213204139](README.assets/image-20260401213204139.png)

![image-20260325225532674](README.assets/image-20260325225532674.png)

### select and highlight

------

![image-20260325225722346](README.assets/image-20260325225722346.png)

![image-20260325225804474](README.assets/image-20260325225804474.png)

## The things we do

### geometric modeling based OCC

sketch-based modeling

### Interactive Widgets

viewcube

clip

### select 

point  select

rect select

### render effects

pbr 

path trace

ssao

transparent

post effects

plans to do:

- [ ] shadow

## how we design it

### Intersective Widgets

![image-20260819112630072](README.assets/image-20260819112630072.png)

## Features

- [x] animation effects to camera perspective adjustment

- [x] pathtracing and PBR

- [x] Render options panel

- [x] Materials panel

- [x] Visibility Control and Selection Interactions

- [x] postprocess effects

- [ ] and plans to do

  
