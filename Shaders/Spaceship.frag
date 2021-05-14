/*
  Original shader: https://www.shadertoy.com/view/3slyRX
  Author: evvvvil (https://www.twitch.tv/evvvvil_)
  Licensed under a CC BY-NC-SA 3.0 license. (https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)
*/

//https://www.shadertoy.com/view/3slyRX


vec2 z,v,e=vec2(.00035,-.00035); float t,tt,b,bb,g,gg;vec3 np,bp,pp,cp,dp,po,no,al,ld;
float bo(vec3 p,vec3 r){p=abs(p)-r;return max(max(p.x,p.y),p.z);}
mat2 r2(float r){return mat2(cos(r),sin(r),-sin(r),cos(r));}
float smin(float a,float b,float h){ float k=clamp((a-b)/h*.5+.5,0.,1.);return mix(a,b,k)-k*(1.-k)*h;}
float noi(vec3 p){
  vec3 f=floor(p),s=vec3(7,157,113);
  p-=f; vec4 h=vec4(0,s.yz,s.y+s.z)+dot(f,s);
  p=p*p*(3.-2.*p);
  h=mix(fract(sin(h)*43758.5),fract(sin(h+s.x)*43758.5),p.x);
  h.xy=mix(h.xz,h.yw,p.y);
  return mix(h.x,h.y,p.z);  
}
vec2 fb( vec3 p, float s )
{
  vec2 h,t=vec2(bo(p,vec3(5,5,2)),s);
  t.x=max(t.x,-bo(p,vec3(3.5,3.5,2)));
  t.x=abs(t.x)-.3;
  t.x=max(t.x,bo(p,vec3(10,10,1)));
  h=vec2(bo(p,vec3(5,5,2)),6);
  h.x=max(h.x,-bo(p,vec3(3.5,3.5,2)));
  h.x=abs(h.x)-.1;
  h.x=max(h.x,bo(p,vec3(10,10,1.4)));
  t=t.x<h.x?t:h;
  h=vec2(length(abs(p.xz)-vec2(2,0))-.2,3);
  t=t.x<h.x?t:h; return t;
}
vec2 mp( vec3 p )
{ 
  bp=p+vec3(0,0,tt*10.);
  np=p+noi(bp*.05)*15.+noi(bp*.5)*1.+noi(bp*4.)*.1+noi(bp*0.01)*20.; 
  vec2 h,t=vec2(np.y+20.,5);
  t.x=smin(t.x,0.75*(length(abs(np.xy-vec2(0,10.+sin(p.x*.1)*10.))-vec2(65,0))-(18.+sin(np.z*.1+tt)*10.)),15.);
  t.x*=0.5;  
  pp=p+vec3(10,15,0);
  pp.x+=sin(p.z*.02+tt/5.)*7.+sin(p.z*.001+20.+tt/100.)*4.;
  bp=abs(pp);bp.xy*=r2(-.785);
  h=vec2(bo(bp-vec3(0,6,0),vec3(2,0.5,1000)),6);
  t=t.x<h.x?t:h;
  h=vec2(bo(bp-vec3(0,6.2,0),vec3(1.,.8,1000)),3);
  t=t.x<h.x?t:h;  
  cp=pp-dp;
  cp.xy*=r2(sin(tt*.4)*.5);  
  h=vec2(length(cp.xy)-(max(-1.,.3+cp.z*.03)),6); 
  h.x=max(h.x,bo(cp+vec3(0,0,25),vec3(10,10,30)));
  g+=0.1/(0.1*h.x*h.x*(20.-abs(sin(abs(cp.z*.1)-tt*3.))*19.7));
  t=t.x<h.x?t:h;
  cp*=1.3;
  for(int i=0;i<3;i++){
    cp=abs(cp)-vec3(-2,0.5,4); 
    cp.xy*=r2(2.0);     
    cp.xz*=r2(.8+sin(cp.z*.1)*.2);     
    cp.yz*=r2(-.8+sin(cp.z*.1)*.2);     
  } 
  h=fb(cp,8.); h.x*=0.5;  t=t.x<h.x?t:h;
  pp.z=mod(pp.z+tt*10.,40.)-20.;
  pp=abs(pp)-vec3(0,20,0);  
  for(int i=0;i<3;i++){
    pp=abs(pp)-vec3(4.2,3,0); 
    pp.xy*=r2(.785); 
    pp.x-=2.;
  }  
  h=fb(pp.zyx,7.); t=t.x<h.x?t:h;
  h=vec2(0.5*bo(abs(pp.zxy)-vec3(7,0,0),vec3(0.1,0.1,1000)),6);
  g+=0.2/(0.1*h.x*h.x*(50.+sin(np.y*np.z*.001+tt*3.)*48.)); t=t.x<h.x?t:h;
  t=t.x<h.x?t:h; return t;
}
vec2 tr( vec3 ro, vec3 rd )
{
  vec2 h,t= vec2(.1);
  for(int i=0;i<128;i++){
    h=mp(ro+rd*t.x);
    if(h.x<.0001||t.x>250.) break;
    t.x+=h.x;t.y=h.y;
  }
  if(t.x>250.) t.y=0.;
  return t;
}
#define a(d) clamp(mp(po+no*d).x/d,0.,1.)
#define s(d) smoothstep(0.,1.,mp(po+ld*d).x/d)
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 uv=(fragCoord.xy/iResolution.xy-0.5)/vec2(iResolution.y/iResolution.x,1);
  tt=mod(iTime+3.,62.82);
  dp=vec3(sin(tt*.4)*4.,20.+sin(tt*.4)*2.,-200.+mod(tt*30.,471.2388));
  vec3 ro=mix(dp-vec3(10,20.+sin(tt*.4)*5.,40),vec3(17,-5,0),ceil(sin(tt*.4))),
  cw=normalize(dp-vec3(10,15,0)-ro), cu=normalize(cross(cw,normalize(vec3(0,1,0)))),cv=normalize(cross(cu,cw)),
  rd=mat3(cu,cv,cw)*normalize(vec3(uv,.5)),co,fo;
  ld=normalize(vec3(.2,.4,-.3));
  co=fo=vec3(.1,.1,.15)-length(uv)*.1-rd.y*.1;
  z=tr(ro,rd);t=z.x;
  if(z.y>0.){
    po=ro+rd*t;
    no=normalize(e.xyy*mp(po+e.xyy).x+e.yyx*mp(po+e.yyx).x+e.yxy*mp(po+e.yxy).x+e.xxx*mp(po+e.xxx).x);
    al=mix(vec3(.4,.0,.1),vec3(.7,.1,.1),cos(bp.y*.08)*.5+.5);
    if(z.y<5.) al=vec3(0);
    if(z.y>5.) al=vec3(1);
    if(z.y>6.) al=clamp(mix(vec3(.0,.1,.4),vec3(.4,.0,.1),sin(np.y*.1+2.)*.5+.5)+(z.y>7.?0.:abs(ceil(cos(pp.x*1.6-1.1))-ceil(cos(pp.x*1.6-1.3)))),0.,1.);
    float dif=max(0.,dot(no,ld)),
    fr=pow(1.+dot(no,rd),4.),
    sp=pow(max(dot(reflect(-ld,no),-rd),0.),30.);
    co=mix(sp+mix(vec3(.8),vec3(1),abs(rd))*al*(a(.1)*a(.4)+.2)*(dif),fo,min(fr,.3));
    co=mix(fo,co,exp(-.0000007*t*t*t));
  }
  fo=mix(vec3(.1,.2,.4),vec3(.1,.1,.5),0.5+0.5*sin(np.y*.1-tt*2.));
  fragColor = vec4(pow(co+g*0.15*mix(fo.xyz,fo.zyx,clamp(sin(tt*.5),-.5,.5)+.5),vec3(.55)),1);
}