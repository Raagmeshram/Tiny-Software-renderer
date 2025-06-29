#include <omp.h>
#include "raylib.h"
#include "raymath.h"
//#include "sex.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#define DELTA_t  0.0167
#define MIN(X, Y) (((X) <= (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y))


typedef struct 
{
  Vector3 Position;
  Vector3 Velocity;
  Vector3 Acceleration;
  Vector3 Angles;
  float FOV;
}Player;
typedef struct {
    int v1, v2, v3;
    int vt1, vt2, vt3;
    int vn1, vn2, vn3;
    int t;  // Indices for vertices
}Faces;

typedef struct 
{ Image Textures[100];
  Vector3 Vertex[300];
  Vector3 Normal[300];
  Vector2 UV[300];
  Faces Face[300];
  int vertex_count;
  int face_count;
  int uv_count;
  int normal_count;
  int tex_count;
}Modal;


Vector3 Subtract(Vector3 input1,Vector3 input2){
  
  return (Vector3){input1.x-input2.x,input1.y-input2.y,input1.z-input2.z};
  
  }

Vector3 FindPoint(Vector3 p1, Vector3 p2, float givenZ) {
    if (p1.z == p2.z) {
        // If the line is parallel to the XY plane, no valid interpolation is possible
        return (Vector3){0, 0, 0}; // Return a default invalid point
    }
    
    // Compute the interpolation factor t
    float t = (givenZ - p1.z) / (p2.z - p1.z);
    
    // Interpolate x and y values using the factor t
    Vector3 result = {
        p1.x + t * (p2.x - p1.x),
        p1.y + t * (p2.y - p1.y),
        givenZ // We set this explicitly as it's the input constraint
    };
    
    return result;
} 


void DrawShadow(Vector3 Master[],Player Cam,int Depthbuffer[640*480],Color pixel){
  Vector3 Vertex[3];
  Vector3 ClippedVert[3];
  
  
  for(int i = 0;i<3;i++){
    Vertex[i]=Master[i];
    Vertex[i]= Subtract(Vertex[i],Cam.Position);
    Vertex[i]=Vector3RotateByAxisAngle(Vertex[i],(Vector3){0,1,0},Cam.Angles.y);
    Vertex[i]=Vector3RotateByAxisAngle(Vertex[i],(Vector3){1,0,0},Cam.Angles.x);
    Vertex[i]=Vector3RotateByAxisAngle(Vertex[i],(Vector3){0,0,1},Cam.Angles.z);
    
  }

  
  for(int i = 0;i<3;i++){
    ClippedVert[i]= Vertex[i];
    ClippedVert[i].x= ((ClippedVert[i].x)*Cam.FOV);
    ClippedVert[i].y=((ClippedVert[i].y)*Cam.FOV);;
    ClippedVert[i].x=ClippedVert[i].x+320;
    ClippedVert[i].y=240-ClippedVert[i].y;
    
    
    
  }
  int x_min;
  int y_min;
  int x_max;
  int y_max;
  
    x_min = (int)MAX(MIN(ClippedVert[0].x,MIN(ClippedVert[1].x,ClippedVert[2].x)),0);
    x_max = (int)MIN(MAX(ClippedVert[0].x,MAX(ClippedVert[1].x,ClippedVert[2].x)),640);
    y_min = (int)MAX(MIN(ClippedVert[0].y,MIN(ClippedVert[1].y,ClippedVert[2].y)),0);
    y_max = (int)MIN(MAX(ClippedVert[0].y,MAX(ClippedVert[1].y,ClippedVert[2].y)),480);
  
  //#pragma omp parallel for collapse(2)
  for(int i =x_min;i<x_max;i++){
    for(int j =y_min;j<y_max;j++){
      if(CheckCollisionPointTriangle((Vector2){i,j}, (Vector2){ClippedVert[0].x,ClippedVert[0].y}, (Vector2){ClippedVert[1].x,ClippedVert[1].y},(Vector2){ClippedVert[2].x,ClippedVert[2].y})){
        Vector3 Barycentre = Vector3Barycenter((Vector3){i,j,0},(Vector3){ClippedVert[0].x,ClippedVert[0].y,0},(Vector3){ClippedVert[1].x,ClippedVert[1].y,0},(Vector3){ClippedVert[2].x,ClippedVert[2].y,0});
        int z = 1000*((Barycentre.x*ClippedVert[0].z)+(Barycentre.y*ClippedVert[1].z)+(Barycentre.z*ClippedVert[2].z));
        
        if(z<Depthbuffer[i+(j*640)] || Depthbuffer[i+(j*640)]== -1){
          DrawPixel(i,j,pixel);
          Depthbuffer[i+(j*640)]=z;
          
        }
        
      }
      
  }
  
  }
  
  
  
}

Vector3 Shadowspace(Vector3 Master,Player Cam){
  Vector3 Vertex;
  
  
  
    Vertex=Master;
    Vertex= Subtract(Vertex,Cam.Position);
    Vertex=Vector3RotateByAxisAngle(Vertex,(Vector3){0,1,0},Cam.Angles.y);
    Vertex=Vector3RotateByAxisAngle(Vertex,(Vector3){1,0,0},Cam.Angles.x);
    Vertex=Vector3RotateByAxisAngle(Vertex,(Vector3){0,0,1},Cam.Angles.z);
    
  

  
  
    
    Vertex.x= ((Vertex.x)*Cam.FOV);
    Vertex.y=((Vertex.y)*Cam.FOV);;
    Vertex.x=Vertex.x+320;
    Vertex.y=240-Vertex.y;
    return Vertex;
    
    
    
  }
  
  
  
  




void Drawcall(Vector3 Master[],Vector3 Normal[],Vector2 UV[],Player Cam,Player Sun,Image *Tex,Color Framebuffer[640*480],int Depthbuffer[640*480],int Shadowbuffer[640*480]){
  Vector3 Vertex[3];
  Vector3 Clippednormal[4];
  Vector2 Clippeduv[4];
  Vector3 ClippedVert[4];
  Vector3 Shadow[3]; 
  Vector3 Clippedshadow[4];
  Vector3 backshadow[3];
  Vector3 Backnormal[3];
  Vector2 Backuv[3]; // to store clipped triangles
  Vector3 BackVertex[3];
  Vector3 Cam_d = {0,0,1};
  Cam_d=Vector3RotateByAxisAngle(Cam_d,(Vector3){0,1,0},Cam.Angles.y);
  Cam_d=Vector3RotateByAxisAngle(Cam_d,(Vector3){1,0,0},Cam.Angles.x);
  Cam_d=Vector3RotateByAxisAngle(Cam_d,(Vector3){0,0,1},Cam.Angles.z);
  
  int front_tri = 0;
  int back_tri = 0;
  /*
  Vector3 Avg_Normal = (Vector3){-(Normal[0].x+Normal[1].x+Normal[2].x)/3,(Normal[0].y+Normal[1].y+Normal[2].y)/3,(Normal[0].z+Normal[1].z+Normal[2].z)/3};
  if (Vector3DotProduct(Avg_Normal,Cam_d)>0)
  {
    return;
  }
  */
  
  for(int i = 0;i<3;i++){
    Vertex[i]=Master[i];
    Shadow[i]=Shadowspace(Vertex[i],Sun);
    Vertex[i]= Subtract(Vertex[i],Cam.Position);
    Vertex[i]=Vector3RotateByAxisAngle(Vertex[i],(Vector3){0,1,0},Cam.Angles.y);
    Vertex[i]=Vector3RotateByAxisAngle(Vertex[i],(Vector3){1,0,0},Cam.Angles.x);
    Vertex[i]=Vector3RotateByAxisAngle(Vertex[i],(Vector3){0,0,1},Cam.Angles.z);
    
    
  }

  for(int i = 0;i<3;i++){
   if(Vertex[i].z>Cam.FOV){
   ClippedVert[front_tri]=Vertex[i];
   Clippednormal[front_tri]=Normal[i];
   Clippeduv[front_tri]=UV[i];
   Clippedshadow[front_tri]=Shadow[i];
    front_tri++;
   }
   else{
    BackVertex[back_tri]=Vertex[i];
    Backnormal[back_tri]=Normal[i];
    Backuv[back_tri]=UV[i];
    backshadow[back_tri]=Shadow[i];
    back_tri++;
   } 
  }
  float x_seg;
  float seg;
  switch (front_tri)
  {
  case 0:
    return;
    break;
  case 1:
    ClippedVert[front_tri]=FindPoint(ClippedVert[0],BackVertex[0],3);

    x_seg = Vector3Distance(ClippedVert[0],ClippedVert[front_tri]);
    seg = Vector3Distance(BackVertex[0],ClippedVert[0]);
    Clippednormal[front_tri] = Vector3Lerp(Clippednormal[0],Backnormal[0],x_seg/seg);
    Clippeduv[front_tri] = Vector2Lerp(Clippeduv[0],Backuv[0],x_seg/seg);
    Clippedshadow[front_tri]=Vector3Lerp(Clippedshadow[0],backshadow[0],x_seg/seg);
    front_tri++;


    ClippedVert[front_tri]=FindPoint(ClippedVert[0],BackVertex[1],3);

    x_seg = Vector3Distance(ClippedVert[0],ClippedVert[front_tri]);
    seg = Vector3Distance(BackVertex[1],ClippedVert[0]);
    Clippednormal[front_tri] = Vector3Lerp(Clippednormal[0],Backnormal[1],x_seg/seg);
    Clippeduv[front_tri] = Vector2Lerp(Clippeduv[0],Backuv[1],x_seg/seg);
    Clippedshadow[front_tri]=Vector3Lerp(Clippedshadow[0],backshadow[1],x_seg/seg);

    
    break;
  case 2:
    ClippedVert[front_tri]=FindPoint(ClippedVert[0],BackVertex[0],3);
    x_seg = Vector3Distance(ClippedVert[0],ClippedVert[front_tri]);
    seg = Vector3Distance(BackVertex[0],ClippedVert[0]);
    Clippednormal[front_tri] = Vector3Lerp(Clippednormal[0],Backnormal[0],x_seg/seg);
    Clippeduv[front_tri] = Vector2Lerp(Clippeduv[0],Backuv[0],x_seg/seg);
    Clippedshadow[front_tri]=Vector3Lerp(Clippedshadow[0],backshadow[0],x_seg/seg);
    front_tri++;

    ClippedVert[front_tri]=FindPoint(ClippedVert[1],BackVertex[0],3);
    x_seg = Vector3Distance(ClippedVert[1],ClippedVert[front_tri]);
    seg = Vector3Distance(BackVertex[0],ClippedVert[1]);
    Clippednormal[front_tri] = Vector3Lerp(Clippednormal[1],Backnormal[0],x_seg/seg);
    Clippeduv[front_tri] = Vector2Lerp(Clippeduv[1],Backuv[0],x_seg/seg);
    Clippedshadow[front_tri]=Vector3Lerp(Clippedshadow[1],backshadow[0],x_seg/seg);
    
    
    
    break;
  case 3:
    front_tri--;
    break;        
  
  }
  
  for(int i = 0;i<=front_tri;i++){
    
    ClippedVert[i].x= ((ClippedVert[i].x/ClippedVert[i].z)*(320/tan(Cam.FOV/2)));
    ClippedVert[i].y=((ClippedVert[i].y/ClippedVert[i].z)*(320/tan(Cam.FOV/2)));;
    ClippedVert[i].x=ClippedVert[i].x+320;
    ClippedVert[i].y=240-ClippedVert[i].y;
    DrawCircle(Clippedshadow[i].x,Clippedshadow[i].y,5,YELLOW);

    
    
  }
  int x_min;
  int y_min;
  int x_max;
  int y_max;
  if(front_tri==3){
    x_min = (int)MAX(MIN(ClippedVert[0].x,MIN(ClippedVert[1].x,MIN(ClippedVert[2].x,ClippedVert[3].x))),0);
    x_max = (int)MIN(MAX(ClippedVert[0].x,MAX(ClippedVert[1].x,MAX(ClippedVert[2].x,ClippedVert[3].x))),640);
    y_min = (int)MAX(MIN(ClippedVert[0].y,MIN(ClippedVert[1].y,MIN(ClippedVert[2].y,ClippedVert[3].y))),0);
    y_max = (int)MIN(MAX(ClippedVert[0].y,MAX(ClippedVert[1].y,MAX(ClippedVert[2].y,ClippedVert[3].y))),480);

  }
  else{
    x_min = (int)MAX(MIN(ClippedVert[0].x,MIN(ClippedVert[1].x,ClippedVert[2].x)),0);
    x_max = (int)MIN(MAX(ClippedVert[0].x,MAX(ClippedVert[1].x,ClippedVert[2].x)),640);
    y_min = (int)MAX(MIN(ClippedVert[0].y,MIN(ClippedVert[1].y,ClippedVert[2].y)),0);
    y_max = (int)MIN(MAX(ClippedVert[0].y,MAX(ClippedVert[1].y,ClippedVert[2].y)),480);
  }
  #pragma omp parallel for collapse(2)
  for(int i =x_min;i<x_max;i++){
    for(int j =y_min;j<y_max;j++){
      if(CheckCollisionPointTriangle((Vector2){i,j}, (Vector2){ClippedVert[0].x,ClippedVert[0].y}, (Vector2){ClippedVert[1].x,ClippedVert[1].y},(Vector2){ClippedVert[2].x,ClippedVert[2].y})){
        Vector3 Barycentre = Vector3Barycenter((Vector3){i,j,0},(Vector3){ClippedVert[0].x,ClippedVert[0].y,0},(Vector3){ClippedVert[1].x,ClippedVert[1].y,0},(Vector3){ClippedVert[2].x,ClippedVert[2].y,0});
        float inversez = (Barycentre.x/ClippedVert[0].z)+(Barycentre.y/ClippedVert[1].z)+(Barycentre.z/ClippedVert[2].z);
        int z = 1000*((Barycentre.x*ClippedVert[0].z)+(Barycentre.y*ClippedVert[1].z)+(Barycentre.z*ClippedVert[2].z));
        


        if(z<Depthbuffer[i+(j*640)] || Depthbuffer[i+(j*640)]== -1){
          float correct_u = ((Clippeduv[0].x*Barycentre.x/ClippedVert[0].z)+(Clippeduv[1].x*Barycentre.y/ClippedVert[1].z)+(Clippeduv[2].x*Barycentre.z/ClippedVert[2].z))/inversez;
          float correct_v = ((Clippeduv[0].y*Barycentre.x/ClippedVert[0].z)+(Clippeduv[1].y*Barycentre.y/ClippedVert[1].z)+(Clippeduv[2].y*Barycentre.z/ClippedVert[2].z))/inversez;
          correct_u = fabs(fmod(correct_u,1));
          correct_v = fabs(fmod(correct_v,1));
          Depthbuffer[i+(j*640)]=z;
          Color pixel = GetImageColor(*Tex,(int)(correct_u*(Tex->width-1)),Tex->height-1-(int)(correct_v*(Tex->height-1)));
         
          Framebuffer[i+(j*640)] = pixel;
        }
        
      }
      if(front_tri==3){
      if(CheckCollisionPointTriangle((Vector2){i,j}, (Vector2){ClippedVert[3].x,ClippedVert[3].y}, (Vector2){ClippedVert[1].x,ClippedVert[1].y},(Vector2){ClippedVert[2].x,ClippedVert[2].y})){
        Vector3 Barycentre = Vector3Barycenter((Vector3){i,j,0},(Vector3){ClippedVert[3].x,ClippedVert[3].y,0},(Vector3){ClippedVert[1].x,ClippedVert[1].y,0},(Vector3){ClippedVert[2].x,ClippedVert[2].y,0});
        float inversez = (Barycentre.x/ClippedVert[3].z)+(Barycentre.y/ClippedVert[1].z)+(Barycentre.z/ClippedVert[2].z);
        int z = 1000*((Barycentre.x*ClippedVert[3].z)+(Barycentre.y*ClippedVert[1].z)+(Barycentre.z*ClippedVert[2].z));
        
        
        if(z<Depthbuffer[i+(j*640)] || Depthbuffer[i+(j*640)]== -1){
          float correct_u = ((Clippeduv[3].x*Barycentre.x/ClippedVert[3].z)+(Clippeduv[1].x*Barycentre.y/ClippedVert[1].z)+(Clippeduv[2].x*Barycentre.z/ClippedVert[2].z))/inversez;
          float correct_v = ((Clippeduv[3].y*Barycentre.x/ClippedVert[3].z)+(Clippeduv[1].y*Barycentre.y/ClippedVert[1].z)+(Clippeduv[2].y*Barycentre.z/ClippedVert[2].z))/inversez;
          correct_u = fabs(fmod(correct_u,1));
          correct_v = fabs(fmod(correct_v,1));
          Depthbuffer[i+(j*640)]=z;
          Color pixel = GetImageColor(*Tex,(int)(correct_u*(Tex->width-1)),Tex->height-1-(int)(correct_v*(Tex->height-1)));;
          
          Framebuffer[i+(j*640)] = pixel;
        }
      }
      }
      
  }
  
  }
  
  
  
}


void loadOBJ(const char *filename,Modal *Mods) {
    Mods->face_count=0;
    Mods->vertex_count=0;
    Mods->normal_count=0;
    Mods->tex_count=0;
    Mods->uv_count=0;
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open %s\n", filename);
        return;
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            Mods->Vertex[Mods->vertex_count++] = (Vector3){x, y, z};
        } 
        else if (strncmp(line, "f ", 2) == 0) {
            int v1, v2, v3;
            int vt1, vt2, vt3;
            int vn1, vn2, vn3;
            
            sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &v1,&vt1,&vn1, &v2,&vt2,&vn2, &v3,&vt3,&vn3);
            Mods->Face[Mods->face_count++] = (Faces){v1 - 1, v2 - 1, v3 - 1,vt1 - 1, vt2 - 1, vt3 - 1,vn1 - 1, vn2 - 1, vn3 - 1,Mods->tex_count - 1};  // Convert 1-based to 0-based indexing
            
        }
        else if (strncmp(line, "vt ", 3) == 0) {
            float x, y;
            sscanf(line + 3, "%f %f", &x, &y);
            Mods->UV[Mods->uv_count++] = (Vector2){x,y};  // Convert 1-based to 0-based indexing
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            float x, y, z;
            sscanf(line + 3, "%f %f %f", &x, &y, &z);
            Mods->Normal[Mods->normal_count++] = (Vector3){x,y,z};  // Convert 1-based to 0-based indexing
        }
        else if (strncmp(line, "usemtl ", 7) == 0) {
            char name[20];
            sscanf(line + 7, "%s", name);
            Mods->Textures[Mods->tex_count++] = LoadImage(TextFormat("%s.png",name));  // Convert 1-based to 0-based indexing
            
            
        }
    }
    fclose(file);
}

void DrawModal(Modal *Mod,Player Cam,Player Sun,Color Framebuffer[640*480],int Depthbuffer[640*480],int Shadowbuffer[640*480]){
  Vector3 vertex[3];
  Vector3 normal[3];
  Vector2 uv[3];
  Image *texture;
  for(int i =0;i<Mod->face_count;i++){
      vertex[0]=Mod->Vertex[Mod->Face[i].v1];
      vertex[1]=Mod->Vertex[Mod->Face[i].v2];
      vertex[2]=Mod->Vertex[Mod->Face[i].v3];
      normal[0]=Mod->Normal[Mod->Face[i].vn1];
      normal[1]=Mod->Normal[Mod->Face[i].vn2];
      normal[2]=Mod->Normal[Mod->Face[i].vn3];
      uv[0]=Mod->UV[Mod->Face[i].vt1];
      uv[1]=Mod->UV[Mod->Face[i].vt2];
      uv[2]=Mod->UV[Mod->Face[i].vt3];
      texture = &Mod->Textures[Mod->Face[i].t];
      Drawcall(vertex,normal,uv,Cam,Sun,texture,Framebuffer,Depthbuffer,Shadowbuffer);
      
  }


}

Modal TransforM(Modal Mod,Vector3 position,Vector3 rotation,Vector3 Scale){
  
  
  for(int i =0;i<Mod.vertex_count;i++){
    Mod.Vertex[i].x=Mod.Vertex[i].x*Scale.x;
    Mod.Vertex[i].y=Mod.Vertex[i].y*Scale.y;
    Mod.Vertex[i].z=Mod.Vertex[i].z*Scale.z;
    Mod.Vertex[i]=Vector3RotateByAxisAngle(Mod.Vertex[i],(Vector3){0,1,0},rotation.y);
    Mod.Vertex[i]=Vector3RotateByAxisAngle(Mod.Vertex[i],(Vector3){1,0,0},rotation.x);
    Mod.Vertex[i]=Vector3RotateByAxisAngle(Mod.Vertex[i],(Vector3){0,0,1},rotation.z);
    Mod.Vertex[i]=Vector3Add(position,Mod.Vertex[i]);
    
      
      
  }
  return Mod;


}
void UpdatePlayer(Player *Cam,Vector2 mouse){
  if(IsKeyDown(KEY_UP)){
    Cam->FOV+=0.1;
    

  }
  if(IsKeyDown(KEY_DOWN)){
    Cam->FOV-=0.1;
   
  }
  
  
  
  if(IsKeyDown(KEY_W)){
    Cam->Position.x-=5*sin(Cam->Angles.y);
    Cam->Position.z+=5*cos(Cam->Angles.y);

  }
  if(IsKeyDown(KEY_S)){
    Cam->Position.x+=5*sin(Cam->Angles.y);
    Cam->Position.z-=5*cos(Cam->Angles.y);
   
  }
   if(IsKeyDown(KEY_A)){
    Cam->Position.x-=5*cos(Cam->Angles.y);
    Cam->Position.z-=5*sin(Cam->Angles.y);
    
    

  }
  if(IsKeyDown(KEY_D)){
    Cam->Position.x+=5*cos(Cam->Angles.y);
    Cam->Position.z+=5*sin(Cam->Angles.y);
   
  }
  if(IsKeyDown(KEY_SPACE)){
    Cam->Velocity.y = 200;
    
   
  }
  if(IsKeyReleased(KEY_SPACE)){
    Cam->Velocity.y = 100;
  }

  Cam->Angles.y -= mouse.x*0.001;
  Cam->Angles.y = fmod(Cam->Angles.y, 2*PI);
  if (Cam->Angles.y < 0) Cam->Angles.y += 2 * PI;
 
  Cam->Angles.x -= mouse.y*0.001;
  Cam->Angles.x = Clamp(Cam->Angles.x,-PI/2,PI/2);

  if(Cam->Position.y<0)Cam->Position.y=0;
  

}

void UpdatePhysics(Player *Cam){
  if(Cam->Position.y>1){
    Cam->Velocity.y+=Cam->Acceleration.y*DELTA_t;

  }
  else if(Cam->Position.y<0){
    Cam->Velocity.y=0;
  }
  Cam->Velocity.x+=Cam->Acceleration.x*DELTA_t;
  Cam->Velocity.z+=Cam->Acceleration.z*DELTA_t;
  Cam->Position.x+=Cam->Velocity.x*DELTA_t;
  Cam->Position.y+=Cam->Velocity.y*DELTA_t;
  Cam->Position.z+=Cam->Velocity.z*DELTA_t;

}



int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1440;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    Color Framebuffer[640*480];
    int* Depthbuffer = (int*)malloc(sizeof(int)*(640*480));
    int* Shadowbuffer = (int*)malloc(sizeof(int)*(640*480));
    Vector2 mouse;
    int fps = 30;
    int zfd=1;
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    Modal ball;
    loadOBJ("clap.obj",&ball);
    //Modal ground;
    //loadOBJ("ground.obj",&ground);
    //Modal ak;
    //loadOBJ("ak.obj",&ak);
    Modal ak2;

    Player Cam;
    Cam.Position = (Vector3){0,100,-500};
    Cam.Velocity = (Vector3){0,0,0};
    Cam.Acceleration = (Vector3){0,-200,0};
    Cam.Angles = (Vector3){0,0,0};
    Cam.FOV = 1.5;

    Player Sun;
    Sun.Position = (Vector3){0,200,0};
    Sun.Angles = (Vector3){-0.2,0.7,0};
    Sun.FOV = 0.7;
    
    Vector3 vertex[3] = {{0,0,500},{200,0,500},{0,200,500}};
    Texture2D ScreenDisplay;
    Image image = GenImageColor(640,480,RED);
    ScreenDisplay = LoadTextureFromImage(image);
    



    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {   
      memset(Framebuffer, 0, sizeof(Framebuffer));
      memset(Depthbuffer, -1, (640*480)*sizeof(int));
      memset(Shadowbuffer, -1, (640*480)*sizeof(int));



        mouse= GetMouseDelta();
        SetMousePosition(720,540);
        UpdatePlayer(&Cam,mouse);
        UpdatePhysics(&Cam);
        if(IsKeyDown(KEY_J)){
          if(fps>0)fps--;
        }

        else if(fps<30){
          fps++;
        }
        if(IsKeyDown(KEY_G))zfd++;
        ak2 = TransforM(ball,(Vector3){0,zfd,0},(Vector3){0,0,0},(Vector3){1,1,1});
        //ak2 = TransforM(ak2,Cam.Position,(Vector3){0,-Cam.Angles.y,Cam.Angles.z});
        //ak2 = TransforM(ak2,(Vector3){0,0,0},(Vector3){0,Cam.Angles.y,Cam.Angles.z});
        
        
        
        
        
        
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            
            
            //DrawModal(&ball,Cam,Sun,Framebuffer,Depthbuffer,Shadowbuffer);
            //DrawModal(&ground,Cam,Sun,Framebuffer,Depthbuffer,Shadowbuffer);
            DrawModal(&ak2,Cam,Sun,Framebuffer,Depthbuffer,Shadowbuffer);
            UpdateTexture(ScreenDisplay,Framebuffer);
            DrawTextureEx(ScreenDisplay,(Vector2){0,0},0,2,WHITE);
            DrawText(TextFormat("Score: %.2f", Cam.Position.x), 20, 20, 20, BLACK);
            DrawText(TextFormat("Score: %.2f", Cam.Position.y), 40, 50, 20, BLACK);
            DrawText(TextFormat("Score: %.2f", Cam.Position.z), 60, 80, 20, BLACK);
            //fuck();
            //DrawCircle(100,10,60,BLUE);
            DrawFPS(10,10);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
