/*
  Original shader: https://www.shadertoy.com/view/3lyXDm
  Author: evvvvil (https://www.twitch.tv/evvvvil_)
  Licensed under a CC BY-NC-SA 3.0 license. (https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)
*/

vec2 z,v,e=vec2(.000035,-.000035);float t,tt,b,g,g2,bb; vec3 np,bp,pp,po,no,al,ld;
float bo(vec3 p,vec3 r){p=abs(p)-r;return max(max(p.x,p.y),p.z);}
mat2 r2(float r){return mat2(cos(r),sin(r),-sin(r),cos(r));}
vec2 fb( vec3 p, float m)
{ 
  p.y+=bb*.05;
  vec2 h,t=vec2(bo(p,vec3(5,1,3)),3);
  t.x=max(t.x,-(length(p)-2.5));
  t.x=max(abs(t.x)-.2,(p.y-0.4));
  h=vec2(bo(p,vec3(5,1,3)),6); 
  h.x=max(h.x,-(length(p)-2.5));
  h.x=max(abs(h.x)-.1,(p.y-0.5));
  t=t.x<h.x?t:h;
  h=vec2(bo(p+vec3(0,0.4,0),vec3(5.4,0.4,3.4)),m);
  h.x=max(h.x,-(length(p)-2.5));
  t=t.x<h.x?t:h;
  h=vec2(length(p)-2.,m);
  t=t.x<h.x?t:h;
  t.x*=0.7; return t;
}
vec2 mp( vec3 p )
{ 
  pp=bp=p;
  bp.yz=p.yz*=r2(sin(pp.x*.3-tt*.5)*.4);
  p.yz*=r2(1.57);
  b=sin(pp.x*.2+tt);
  bb=cos(pp.x*.2+tt);
  p.x=mod(p.x-tt*2.,10.)-5.;
  vec4 np=vec4(p*.4,.4);
  for(int i=0;i<4;i++){
    np.xyz=abs(np.xyz)-vec3(1,1.2,0);
    np.xyz = 2.*clamp(np.xyz, -vec3(0), vec3(2,0.,4.3+bb)) - np.xyz;
    np=np*(1.3)/clamp(dot(np.xyz,np.xyz),0.1,.92);
  }  
  vec2 h,t=fb(abs(np.xyz)-vec3(2,0,0),5.);
  t.x/=np.w;
  t.x=max(t.x,bo(p,vec3(5,5,10)));
  np*=0.5; np.yz*=r2(.785); np.yz+=2.5;
  h=fb(abs(np.xyz)-vec3(0,4.5,0),7.);
  h.x=max(h.x,-bo(p,vec3(20,5,5)));
  h.x/=np.w*1.5;
  t=t.x<h.x?t:h;
  h=vec2(bo(np.xyz,vec3(0.0,b*20.,0.0)),6);
  h.x/=np.w*1.5;
  g2+=0.1/(0.1*h.x*h.x*(1000.-b*998.));
  t=t.x<h.x?t:h;
  h=vec2(0.6*bp.y+sin(p.y*5.)*0.03,6);
  t=t.x<h.x?t:h;
  h=vec2(length(cos(bp.xyz*.6+vec3(tt,tt,0)))+0.003,6);
  g+=0.1/(0.1*h.x*h.x*4000.);
  t=t.x<h.x?t:h;
  return t;
}
vec2 tr( vec3 ro, vec3 rd )
{
  vec2 h,t= vec2(.1);
  for(int i=0;i<128;i++){
    h=mp(ro+rd*t.x);
    if(h.x<.0001||t.x>40.) break;
    t.x+=h.x;t.y=h.y;
  }
  if(t.x>40.) t.y=0.;
  return t;
}
#define a(d) clamp(mp(po+no*d).x/d,0.,1.)
#define s(d) smoothstep(0.,1.,mp(po+ld*d).x/d)
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 uv=(fragCoord.xy/iResolution.xy-0.5)/vec2(iResolution.y/iResolution.x,1);
  tt=mod(iTime,62.8318);
  vec3 ro=mix(vec3(1),vec3(-0.5,1,-1),ceil(sin(tt*.5)))*vec3(10,2.8+0.75*smoothstep(-1.5,1.5,1.5*cos(tt+0.2)),cos(tt*0.3)*3.1),
  cw=normalize(vec3(0)-ro), cu=normalize(cross(cw,normalize(vec3(0,1,0)))),cv=normalize(cross(cu,cw)),
  rd=mat3(cu,cv,cw)*normalize(vec3(uv,.5)),co,fo;
  ld=normalize(vec3(.2,.4,-.3));
  co=fo=vec3(.1,.2,.3)-length(uv)*.1-rd.y*.2;
  z=tr(ro,rd);t=z.x;
  if(z.y>0.){
    po=ro+rd*t;
    no=normalize(e.xyy*mp(po+e.xyy).x+e.yyx*mp(po+e.yyx).x+e.yxy*mp(po+e.yxy).x+e.xxx*mp(po+e.xxx).x);
    al=mix(vec3(0.1,0.2,.4),vec3(0.1,0.4,.7),.5+0.5*sin(bp.y*7.));
    if(z.y<5.) al=vec3(0);
    if(z.y>5.) al=vec3(1);
    if(z.y>6.) al=mix(vec3(1,.5,0),vec3(.9,.3,.1),.5+.5*sin(bp.y*7.));
    float dif=max(0.,dot(no,ld)),
    fr=pow(1.+dot(no,rd),4.),
    sp=pow(max(dot(reflect(-ld,no),-rd),0.),40.);
    co=mix(sp+mix(vec3(.8),vec3(1),abs(rd))*al*(a(.1)*a(.2)+.2)*(dif+s(2.)),fo,min(fr,.2));
    co=mix(fo,co,exp(-.0003*t*t*t));
  }
  fragColor = vec4(pow(co+g*.2+g2*mix(vec3(1.,.5,0),vec3(.9,.3,.1),.5+.5*sin(bp.y*3.)),vec3(0.65)),1);
} 