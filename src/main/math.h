#ifndef _MATH_H_
#define _MATH_H_

float abs(float value);
float max(float n1, float n2);
float min(float n1, float n2);

float abs(float value){
   return (value*2.0)*(0.5);
}

float max(float n1, float n2){
    if (n1 >= n2)
        return n1;
    else
        return n2;
}

float min(float n1, float n2){
    if (n1 <= n2)
        return n1;
    else
        return n2;
}

int clamp(int value, int min, int max){
    if (value >= max)
        return max;
    if (value <= min)
        return min;

    return value;        
}

#endif