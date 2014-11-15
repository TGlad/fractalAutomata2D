#include "Evolver.h"
#include "Image.h"
// #define EXTREME_VARIATIONS
extern int g_type;

Evolver::Evolver(int depth)
{
  this->depth = depth;
  bitmaps = new Image*[depth+1];
  for (int size = 2, i=1; i<=depth; size *= 2, i++)
  {
    bitmaps[i] = new Image(size+2, size+2); // store each pixel
    bitmaps[i]->clear();
  }
  reset();
}

void Evolver::updateCovolveData()
{
  parentWeight = (float)parentW;
  for (int i = 0; i<3; i++)
    for (int j = 0; j<3; j++)
      siblingWeights[i][j] = (i<j) ? (float)siblingWs[j][i] : (float)siblingWs[i][j];
  for (int i = 0; i<2; i++)
    for (int j = 0; j<2; j++)
      childWeights[i][j] = (i<j) ? (float)childWs[j][i] : (float)childWs[i][j];

  float total = abs(parentWeight);
  for (int i = 0; i<3; i++)
    for (int j = 0; j<3; j++)
      total += abs(siblingWeights[i][j]);
  for (int i = 0; i<2; i++)
    for (int j = 0; j<2; j++)
      total += abs(childWeights[i][j]);
  totalWeight = total;
}

// returns number in range -16 to 16, not even distribution though
int random16() 
{
  int e1 = random() > 0.0f ? 1 : 0;
  int e2 = random() > 0.0f ? 1 : 0;
  int e4 = random() > 0.0f ? 1 : 0;
  int e8 = random() > 0.0f ? 1 : 0;
  int e16= random() > 0.0f ? 1 : 0;
  return -16 + e1 + 2*e2 + 4*e4 + 8*e8 + 16*e16;
}

void flipBit(int& val, int i)
{
  if (val & (1<<i))
    val = val & ~(1<<i);
  else
    val = val | (1<<i);
}

// returns number in range -16 to 16, not even distribution though
void covolveRandom(int& val, float threshold) 
{
  int value = val + 16; // 0 to 32
  for (int i = 0; i<5; i++)
    if (random() > threshold)
      flipBit(value, i);
  val = value - 16;
}

void Evolver::reset()
{
  frame = 0;

  // type 3
  for (int i = 0; i<1<<9; i++)
    siblingMasks[i] = random()>0;
  for (int i = 0; i<1<<4; i++)
    parentMasks[i] = random()>0;
  for (int i = 0; i<1<<4; i++)
    childMasks[i] = random()>0;

  for (int i = 0; i<32; i++)
    totalMasks[i] = random() > 0 ? 1 : 0;

  // type 6
  for (int i = 0; i<6; i++)
    for (int j = 0; j<3; j++)
      for (int k = 0; k<3; k++)
      {
        parentsAdd[i][j][k] = random() > 0 ? 1 : 0;
        parentsRemove[i][j][k] = random() > 0 ? 1 : 0;
      }

  // type 7
  covolveScale = 2.0f;
  parentW = random16();
  for (int i = 0; i<3; i++)
    for (int j = 0; j<3; j++)
      siblingWs[i][j] = random16();
  for (int i = 0; i<2; i++)
    for (int j = 0; j<2; j++)
      childWs[i][j] = random16();
  updateCovolveData();

  randomise();
}

void Evolver::randomiseMasks(const Evolver& master, float percentVariation)
{
  float threshold = 1.0f - 2.0f*0.01f*percentVariation;
  for (int i = 0; i<1<<9; i++)
  {
    siblingMasks[i] = master.siblingMasks[i];
    if (random() > threshold)
      siblingMasks[i] = !siblingMasks[i];
  }
  for (int i = 0; i<1<<4; i++)
  {
    parentMasks[i] = master.parentMasks[i];
    if (random() > threshold)
      parentMasks[i] = !parentMasks[i];
  }
  for (int i = 0; i<1<<4; i++)
  {
    childMasks[i] = master.childMasks[i];
    if (random() > threshold)
      childMasks[i] = !childMasks[i];
  }
  frame = 0;

  for (int i = 0; i<32; i++)
  {
    totalMasks[i] = master.totalMasks[i];
    if (random() > threshold)
      totalMasks[i] = 1-totalMasks[i];
  }
  for (int i = 0; i<6; i++)
    for (int j = 0; j<3; j++)
      for (int k = 0; k<3; k++)
      {
        parentsAdd[i][j][k] = master.parentsAdd[i][j][k];
        parentsRemove[i][j][k] = master.parentsRemove[i][j][k];
        if (random() > threshold)
          parentsAdd[i][j][k] = !parentsAdd[i][j][k];
        if (random() > threshold)
          parentsRemove[i][j][k] = !parentsRemove[i][j][k];
      }

  parentW = master.parentW;
  covolveRandom(parentW, threshold);
  for (int i = 0; i<3; i++)
    for (int j = 0; j<3; j++)
    {
      siblingWs[i][j] = master.siblingWs[i][j];
      covolveRandom(siblingWs[i][j], threshold);
    }
  for (int i = 0; i<2; i++)
    for (int j = 0; j<2; j++)
    {
      childWs[i][j] = master.childWs[i][j];
      covolveRandom(childWs[i][j], threshold);
    }
  updateCovolveData();
}

void Evolver::randomise()
{
  // initialise the bitmaps to some random image:
  // This is a recursive process, generating the data procedurally as we go deeper in detail level.
  for (int level = 2; level<=depth; level++)
  {
    int size = 1<<level;
    for (int i = 0; i<size; i++)
      for (int j = 0; j<size; j++)
        bitmaps[level]->setPixel(i, j, random() > 0.5f ? 128 : 0);
  }
}

void Evolver::read(FILE* fp)
{
  switch(g_type)
  {
  case(1):
    for (int i = 0; i<16; i++)
      fread(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(2):
    for (int i = 0; i<17; i++)
      fread(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(3):
    for (int i = 0; i<1<<9; i++)
      fread(&siblingMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fread(&parentMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fread(&childMasks[i], sizeof(bool), 1, fp);
    break;
  case(4):
    for (int i = 0; i<32; i++)
      fread(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(5):
    for (int i = 0; i<128; i++)
      fread(&siblingMasks[i], sizeof(int), 1, fp);
    break;
  case(6):
    for (int i = 0; i<6; i++)
      for (int j = 0; j<3; j++)
        for (int k = 0; k<3; k++)
        {
          fread(&parentsAdd[i][j][k], sizeof(bool), 1, fp);
          fread(&parentsRemove[i][j][k], sizeof(bool), 1, fp);
        }
    break;
  case(7):
    fread(&covolveScale, sizeof(float), 1, fp);
    fread(&parentW, sizeof(int), 1, fp);
    for (int i = 0; i<3; i++)
      for (int j = 0; j<3; j++)
        fread(&siblingWs[i][j], sizeof(int), 1, fp);
    for (int i = 0; i<2; i++)
      for (int j = 0; j<2; j++)
        fread(&childWs[i][j], sizeof(int), 1, fp);
    updateCovolveData();
    break;
  default:
    return;
  }
  frame = 0;
}

void Evolver::write(FILE* fp)
{
  switch(g_type)
  {
  case(1):
    for (int i = 0; i<16; i++)
      fwrite(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(2):
    for (int i = 0; i<17; i++)
      fwrite(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(3):
    for (int i = 0; i<1<<9; i++)
      fwrite(&siblingMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fwrite(&parentMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fwrite(&childMasks[i], sizeof(bool), 1, fp);
    break;
  case(4):
    for (int i = 0; i<32; i++)
      fwrite(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(5):
    for (int i = 0; i<128; i++)
      fwrite(&siblingMasks[i], sizeof(int), 1, fp);
    break;
  case(6):
    for (int i = 0; i<6; i++)
      for (int j = 0; j<3; j++)
        for (int k = 0; k<3; k++)
        {
          fwrite(&parentsAdd[i][j][k], sizeof(bool), 1, fp);
          fwrite(&parentsRemove[i][j][k], sizeof(bool), 1, fp);
        }
    break;
  case(7):
    fwrite(&covolveScale, sizeof(float), 1, fp);
    fwrite(&parentW, sizeof(int), 1, fp);
    for (int i = 0; i<3; i++)
      for (int j = 0; j<3; j++)
        fwrite(&siblingWs[i][j], sizeof(int), 1, fp);
    for (int i = 0; i<2; i++)
      for (int j = 0; j<2; j++)
        fwrite(&childWs[i][j], sizeof(int), 1, fp);
    break;
  default:
    break;
  }
}


void Evolver::set(const Evolver& evolver)
{
  for (int i = 0; i<1<<9; i++)
    siblingMasks[i] = evolver.siblingMasks[i];
  for (int i = 0; i<1<<4; i++)
    parentMasks[i] = evolver.parentMasks[i];
  for (int i = 0; i<1<<4; i++)
    childMasks[i] = evolver.childMasks[i];

  frame = 0;
  for (int i = 0; i<32; i++)
    totalMasks[i] = evolver.totalMasks[i];

  for (int i = 0; i<6; i++)
    for (int j = 0; j<3; j++)
      for (int k = 0; k<3; k++)
      {
        parentsAdd[i][j][k] = evolver.parentsAdd[i][j][k];
        parentsRemove[i][j][k] = evolver.parentsRemove[i][j][k];
      }

  covolveScale = evolver.covolveScale;
  parentW = evolver.parentW;
  for (int i = 0; i<3; i++)
    for (int j = 0; j<3; j++)
      siblingWs[i][j] = evolver.siblingWs[i][j];
  for (int i = 0; i<2; i++)
    for (int j = 0; j<2; j++)
      childWs[i][j] = evolver.childWs[i][j];
  updateCovolveData();
}

void Evolver::draw()
{
  bitmaps[depth]->generateTexture();
  bitmaps[depth]->draw();
}

bool Evolver::getNewValue(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  int xx = X-dirX;
  int pattern1 = 0;
  int pattern2 = 0;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      if (bitmaps[level]->isSet(xx, yy))
      {
        pattern1 += 1<<(x+3*y);
        pattern2 += 1<<(y+3*x);
      }
      yy += dirY;
    }
    xx += dirX;
  }
  if (!siblingMasks[min(pattern1, pattern2)])
    return false;

  xx = X/2;
  pattern1 = 0;
  pattern2 = 0;
  if (level > 0)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y/2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level-1]->isSet(xx, yy))
        {
          pattern1 += 1<<(x+2*y);
          pattern2 += 1<<(y+2*x);
        }
        yy += dirY;
      }
      xx += dirX;
    }
  }
  if (parentMasks[min(pattern1, pattern2)]) // the min ensures we remain symettric
    return true;

  pattern1 = 0;
  pattern2 = 0;
  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level+1]->isSet(xx, yy))
        {
          pattern1 += 1<<(x+2*y);
          pattern2 += 1<<(y+2*x);
        }
        yy += dirY;
      }
      xx += dirX;
    }
  }
  if (childMasks[min(pattern1, pattern2)])
    return true;
  return false;
}

float Evolver::getNewValueCovolve(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  float result = 0.0f;

  int xx = X-dirX;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      float value = -1.0f + (float)(bitmaps[level]->pixel(xx, yy)&255) / 128.0f; // -1 to 1
      result += value * siblingWeights[x][y];
      yy += dirY;
    }
    xx += dirX;
  }

  if (level > 0)
  {
    float value = -1.0f + (float)(bitmaps[level-1]->pixel(X/2, Y/2)&255) / 128.0f; // -1 to 1
    result += value * parentWeight;
  }

  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        float value = -1.0f + (float)(bitmaps[level+1]->pixel(xx, yy)&255) / 128.0f; // -1 to 1
        result += value * childWeights[x][y];
        yy += dirY;
      }
      xx += dirX;
    }
  }
  return covolveScale * result / totalWeight;
}

bool Evolver::checkAddRemove(int level, int X, int Y, bool addRemove[6][3][3], int i, bool flip)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;
  int xx = X/2 - dirX;
  for (int x = 0; x<3; x++, xx += dirX)
  {
    int yy = Y/2 - dirY;
    for (int y = 0; y<3; y++, yy += dirY)
    {
      if (x==1 && y==1)
        continue;
      if ((addRemove == parentsRemove && x==2 && y==2) || (addRemove == parentsAdd && x==0 && y==0))
        continue;
      bool mask = flip ? addRemove[i][y][x] : addRemove[i][x][y];
      if (bitmaps[level-1]->isSet(xx, yy) != mask)
        return false;
    }
  }
  return true;
}

bool Evolver::getNewValueParentsOnly(int level, int X, int Y)
{
  ASSERT(level > 0);
  if (bitmaps[level-1]->isSet(X/2, Y/2)) // then prepare to remove
  {
    for (int i = 0; i<6; i++)
      if (checkAddRemove(level, X, Y, parentsRemove, i, false) || checkAddRemove(level, X, Y, parentsRemove, i, true))
        return false;
    return true;
  }
  else
  {
    for (int i = 0; i<6; i++)
      if (checkAddRemove(level, X, Y, parentsAdd, i, false) || checkAddRemove(level, X, Y, parentsAdd, i, true))
        return true;
    return false;
  }
}

bool Evolver::getNewValue2(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  int xx = X-dirX;
  // First, num neighbours
  int pattern1 = 0;
  int pattern2 = 0;
  int numNeighbours = 0;
  bool centreSet = false;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      if (bitmaps[level]->isSet(xx, yy))
      {
        if (x==1 && y==1)
          centreSet = true;
        else
          numNeighbours++;
      }
      yy += dirY;
    }
    xx += dirX;
  }

  xx = X/2;
  bool hasParent = level > 0 ? bitmaps[level-1]->isSet(X/2, Y/2) : 0;

  int numChildren = 0;
  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level+1]->isSet(xx, yy))
          numChildren++;
        yy += dirY;
      }
      xx += dirX;
    }
  }
  return siblingMasks[((numNeighbours + numChildren*8)*2 + (int)hasParent) * 2 + (int)centreSet];
}

bool Evolver::getNewValueSimple(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  int xx = X-dirX;
  // First, num neighbours
  int pattern1 = 0;
  int pattern2 = 0;
  int numNeighbours = 0;
  bool centreSet = false;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      if (bitmaps[level]->isSet(xx, yy))
      {
        if (g_type != 2 && x==1 && y==1)
          centreSet = true;
        else
          numNeighbours++;
      }
      yy += dirY;
    }
    xx += dirX;
  }

  xx = X/2;
  pattern1 = 0;
  pattern2 = 0;
  if (level > 0)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y/2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level-1]->isSet(xx, yy))
        {
          if (x==0 && y==0)
            numNeighbours++;
          else if (x==1 && y==1)
            numNeighbours++;
          else
            numNeighbours++;
        }
        yy += dirY;
      }
      xx += dirX;
    }
  }

  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level+1]->isSet(xx, yy))
          numNeighbours++;
        yy += dirY;
      }
      xx += dirX;
    }
  }
  if (g_type == 4)
    return totalMasks[centreSet ? numNeighbours : numNeighbours + 16]!=0;
  else
    return totalMasks[numNeighbours]!=0;
}


void Evolver::update()
{
  // This performs the covolution
  frame++;
  if (g_type == 6)
  {
    for (int level = 3; level<=depth; level++)
    {
      int size = 1<<level;
      for (int i = 0; i<size; i++)
      {
        for (int j = 0; j<size; j++)
        {
          int& pixel = bitmaps[level]->pixel(i, j);
          if (getNewValueParentsOnly(level, i, j))
            pixel |= 256;
          bitmaps[level]->pixel(i, j) >>= 1;
        }
      }
    }
    return;
  }
  // I need to get the timing algorithm right here...
  int currentLevel = -1;
  for (int i = 0; i<32 && currentLevel==-1; i++)
    if ((1<<i) & frame)
      currentLevel = i; // should go 0,1,0,2,0,1,0,3,...
  currentLevel = depth - currentLevel; // so most common is the highest detail.
  if (currentLevel == 1)
  {
    currentLevel = depth;
    frame = 1;
  }
  ASSERT(currentLevel <= depth && currentLevel > 1);

  int size = 1<<currentLevel;
  if (g_type == 7)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        float result = clamped(getNewValueCovolve(currentLevel, i, j), -1.0f, 1.0f);
        pixel |= ((int)((result + 1.0f) * 127.0f)) << 8;
      }
    }
    for (int i = 0; i<size; i++)
      for (int j = 0; j<size; j++)
        bitmaps[currentLevel]->pixel(i, j) >>= 8;
    return;
  }
  if (g_type < 3 || g_type == 4)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        if (getNewValueSimple(currentLevel, i, j))
          pixel |= 256;
      }
    }
  }
  else if (g_type == 3)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        if (getNewValue(currentLevel, i, j))
          pixel |= 256;
      }
    }
  }
  else if (g_type == 5)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        if (getNewValue2(currentLevel, i, j))
          pixel |= 256;
      }
    }
  }
        
  for (int i = 0; i<size; i++)
    for (int j = 0; j<size; j++)
      bitmaps[currentLevel]->pixel(i, j) >>= 1;
}