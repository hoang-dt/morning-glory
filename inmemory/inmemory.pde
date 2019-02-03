int LeftMargin = 40;
int TopMargin = 40;
int Spacing = 130;
int PointSize = 16;
int SelectX = 0;
int SelectY = 0;

class Block {
  int level = 0;
  int numPointsX = 2;
  int numPointsY = 2;
  int coordX = 0;
  int coordY = 0;
  float startX = 0;
  float startY = 0;
  float spacingX = Spacing;
  float spacingY = Spacing;
  
  void Draw() {
    fill(0, 36, 81);    
    noStroke();
    for (int i = 0; i < numPointsX; ++i) {
      for (int j = 0; j < numPointsY; ++j) {
        ellipse(startX + i * spacingX, startY + j * spacingY, PointSize, PointSize);
      }
    }
  }
  
  void refinePrimary() {
    ++level;
    if (level % 2 == 1) {
      numPointsX *= 2;
      spacingX /= 2;
    } else {
      numPointsY *= 2;
      spacingY /= 2;
    }
    print("refind primary");
  }
  
  void refineSecondary() {
    ++level;
    if (level % 2 == 1) {
      numPointsX += numPointsX - 1;
      spacingX /= 2;      
    } else {
      numPointsY += numPointsY - 1;
      spacingY /= 2;
    }
    print("refind secondary");
  }
}

Block[] Blocks;

void setup() {
  size(1000, 1000);
  Blocks = new Block[49];
  for (int i = 0; i < 49; ++i) {
    Blocks[i] = new Block();
  }
  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j < 7; ++j) {
      Blocks[j * 7 + i].startX = LeftMargin + i * Spacing;
      Blocks[j * 7 + i].startY = TopMargin + j * Spacing;
      Blocks[j * 7 + i].coordX = i;
      Blocks[j * 7 + i].coordY = j;
    }
  }
}

void keyPressed() {
  print("key pressed");
  if (keyCode == UP) {
    SelectY = (SelectY - 2) % 7;
  } else if (keyCode == DOWN) {
    SelectY = (SelectY + 2) % 7;
  } else if (keyCode == LEFT) {
    SelectX = (SelectX - 2) % 7;
  } else if (keyCode == RIGHT) {
    SelectX = (SelectX + 2) % 7;
  }
  if (keyCode == ENTER) {
    Blocks[SelectY * 7 + SelectX].refinePrimary();
    if (SelectX < 7) {
      Blocks[SelectY * 7 + (SelectX + 1)].refineSecondary();
    }
    if (SelectY < 7) {
      Blocks[(SelectY + 1) * 7 + SelectX].refineSecondary();
    }
    if (SelectX < 7 && SelectY < 7) {
      Blocks[(SelectY + 1) * 7 + SelectX + 1].refineSecondary();
    }
  }  
}

void draw() {
  background(255, 255, 255);
  fill(255, 100, 100);
  rect(LeftMargin + SelectX * Spacing, TopMargin + SelectY * Spacing, Spacing, Spacing);
  for (int i = 0; i < 49; ++i) {
    Blocks[i].Draw();
  }  
}
