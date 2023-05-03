#version 140

#define DEFAULT_SIZE 15
#define PLANES_SIZE 5

uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_mouse;
uniform vec3 u_camera_pos;
uniform vec2 u_seed1;
uniform vec2 u_seed2;
uniform sampler2D u_sample;
uniform float u_sample_part;

uniform float sun_brightness;

uniform vec3 ulight_pos;

uniform vec4 spheres_pos[DEFAULT_SIZE];
uniform vec4 spheres_col[DEFAULT_SIZE];

uniform vec3 boxes_pos[DEFAULT_SIZE];
uniform vec3 boxes_size[DEFAULT_SIZE];
uniform vec4 boxes_col[DEFAULT_SIZE];

uniform vec3 planes_norm[PLANES_SIZE];
uniform vec4 planes_col[PLANES_SIZE];

uniform vec4 cones_up_point[DEFAULT_SIZE];
uniform vec4 cones_down_point[DEFAULT_SIZE];
uniform vec4 cones_col[DEFAULT_SIZE];

uniform vec4 cyl_up_point[DEFAULT_SIZE];
uniform vec3 cyl_down_point[DEFAULT_SIZE];
uniform vec4 cyl_col[DEFAULT_SIZE];

const float MAX_DIST = 99999.0; 
const float eps = 0.0001;

/* functions borrowed from extrenal source */

uvec4 R_STATE;

uint TausStep(uint z, int S1, int S2, int S3, uint M)
{
	uint b = (((z << S1) ^ z) >> S2);
	return (((z & M) << S3) ^ b);	
}

uint LCGStep(uint z, uint A, uint C)
{
	return (A * z + C);	
}

vec2 hash22(vec2 p)
{
	p += u_seed1.x;
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yzx+33.33);
	return fract((p3.xx+p3.yz)*p3.zy);
}

float random()
{
	R_STATE.x = TausStep(R_STATE.x, 13, 19, 12, uint(4294967294));
	R_STATE.y = TausStep(R_STATE.y, 2, 25, 4, uint(4294967288));
	R_STATE.z = TausStep(R_STATE.z, 3, 11, 17, uint(4294967280));
	R_STATE.w = LCGStep(R_STATE.w, uint(1664525), uint(1013904223));
	return 2.3283064365387e-10 * float((R_STATE.x ^ R_STATE.y ^ R_STATE.z ^ R_STATE.w));
}

vec3 randomOnSphere() {
	vec3 rand = vec3(random(), random(), random());
	float theta = rand.x * 2.0 * 3.14159265;
	float v = rand.y;
	float phi = acos(2.0 * v - 1.0);
	float r = pow(rand.z, 1.0 / 3.0);
	float x = r * sin(phi) * cos(theta);
	float y = r * sin(phi) * sin(theta);
	float z = r * cos(phi);
	return vec3(x, y, z);
}


/* end of borrowed functions */


mat2 rot( float a ) {

    return mat2( cos(a), -sin(a), sin(a), cos(a) );

}

vec3 GetSky( vec3 rd, vec3 light_pos ) {

    vec3 sky_col = vec3( 0.3, 0.6, 1.0 ); //blue skyes
    vec3 sun = vec3( 0.95, 0.9, 1.0 ); // sun
    sun *= pow ( max( 0.0, dot(rd, -light_pos) ), 256.0 );
    sky_col *= max ( 0.0, dot( -light_pos, vec3( 0.0, 0.0, -1.0 )));
    return clamp( sun + sky_col * sun_brightness, 0.0 , 1.0  );

}

vec2 SphIntersect( in vec3 ro, in vec3 rd, float ra ) {   // func that seek ray intersection
    
    vec3 oc = ro ;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - ra*ra;
    float h = b*b - c;
    
    if( h < 0.0 )
        return vec2( -1.0 ); // no intersection
    
    h = sqrt( h );
    
    return vec2( -b - h, -b + h );
}

vec2 boxIntersection( in vec3 ro, in vec3 rd, in vec3 boxSize, out vec3 outNormal ) {
    
    vec3 m = 1.0 / rd; // can precompute if traversing a set of aligned boxes
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN > tF || tF < 0.0) return vec2(-1.0); // no intersection
	outNormal = -sign(rd) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);
    return vec2( tN, tF );

}

float plaIntersect( in vec3 ro, in vec3 rd, in vec4 p ) {
    return -(dot(ro, p.xyz) + p.w)/dot( rd ,p.xyz );
}

// cone defined by extremes pa and pb, and radious ra and rb
//Only one square root and one division is emplyed in the worst case. dot2(v) is dot(v,v)
vec4 coneIntersect( in vec3  ro, in vec3  rd, in vec3  pa, in vec3  pb, in float ra, in float rb ) {
    vec3  ba = pb - pa;
    vec3  oa = ro - pa;
    vec3  ob = ro - pb;
    float m0 = dot(ba,ba);
    float m1 = dot(oa,ba);
    float m2 = dot(rd,ba);
    float m3 = dot(rd,oa);
    float m5 = dot(oa,oa);
    float m9 = dot(ob,ba); 
    
    // caps
    if( m1<0.0 )
    {
        if( dot(oa*m2-rd*m1, oa*m2-rd*m1)<(ra*ra*m2*m2) ) // delayed division
            return vec4(-m1/m2,-ba*inversesqrt(m0));
    }
    else if( m9>0.0 )
    {
    	float t = -m9/m2;                     // NOT delayed division
        if( dot(ob+rd*t, ob+rd*t)<(rb*rb) )
            return vec4(t,ba*inversesqrt(m0));
    }
    
    // body
    float rr = ra - rb;
    float hy = m0 + rr*rr;
    float k2 = m0*m0    - m2*m2*hy;
    float k1 = m0*m0*m3 - m1*m2*hy + m0*ra*(rr*m2*1.0        );
    float k0 = m0*m0*m5 - m1*m1*hy + m0*ra*(rr*m1*2.0 - m0*ra);
    float h = k1*k1 - k2*k0;
    if( h<0.0 ) return vec4(-1.0); //no intersection
    float t = (-k1-sqrt(h))/k2;
    float y = m1 + t*m2;
    if( y<0.0 || y>m0 ) return vec4(-1.0); //no intersection
    return vec4(t, normalize(m0*(m0*(oa+t*rd)+rr*ba*ra)-ba*hy*y));
}


float roundedboxIntersect( in vec3 ro, in vec3 rd, in vec3 size, in float rad ) {
    // bounding box
    vec3 m = 1.0/rd;
    vec3 n = m*ro;
    vec3 k = abs(m)*(size+rad);
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return -1.0;
    float t = tN;

    // convert to first octant
    vec3 pos = ro+t*rd;
    vec3 s = sign(pos);
    ro  *= s;
    rd  *= s;
    pos *= s;
        
    // faces
    pos -= size;
    pos = max( pos.xyz, pos.yzx );
    if( min(min(pos.x,pos.y),pos.z) < 0.0 ) return t;

    // some precomputation
    vec3 oc = ro - size;
    vec3 dd = rd*rd;
    vec3 oo = oc*oc;
    vec3 od = oc*rd;
    float ra2 = rad*rad;

    t = 1e20;        

    // corner
    {
    float b = od.x + od.y + od.z;
    float c = oo.x + oo.y + oo.z - ra2;
    float h = b*b - c;
    if( h>0.0 ) t = -b-sqrt(h);
    }
    // edge X
    {
        float a = dd.y + dd.z;
        float b = od.y + od.z;
        float c = oo.y + oo.z - ra2;
        float h = b*b - a*c;
        if( h>0.0 ) {
            h = (-b-sqrt(h))/a;
            if( h>0.0 && h<t && abs(ro.x+rd.x*h)<size.x ) t = h;
        }
    }
    // edge Y
    {
        float a = dd.z + dd.x;
        float b = od.z + od.x;
        float c = oo.z + oo.x - ra2;
        float h = b*b - a*c;
        if( h>0.0 ) {
            h = (-b-sqrt(h))/a;
            if( h>0.0 && h<t && abs(ro.y+rd.y*h)<size.y ) t = h;
        }
    }
    // edge Z
    {
        float a = dd.x + dd.y;
        float b = od.x + od.y;
        float c = oo.x + oo.y - ra2;
        float h = b*b - a*c;
        if( h>0.0 ) {
            h = (-b-sqrt(h))/a;
            if( h>0.0 && h<t && abs(ro.z+rd.z*h)<size.z ) t = h;
        }
    }

    if( t>1e19 ) t=-1.0;
    
    return t;
}

vec3 roundedboxNormal( in vec3 pos, in vec3 siz, in float rad ) {
    return sign(pos)*normalize(max(abs(pos)-siz,0.0));
}

float diskIntersect( in vec3 ro, in vec3 rd, vec3 c, vec3 n, float r ) {
    vec3  o = ro - c;
    float t = -dot(n,o)/dot(rd,n);
    vec3  q = o + rd*t;
    return (dot(q,q)<r*r) ? t : -1.0;
}

vec4 cylIntersect( in vec3 ro, in vec3 rd, in vec3 a, in vec3 b, float ra ) {
    vec3  ba = b  - a;
    vec3  oc = ro - a;
    float baba = dot(ba,ba);
    float bard = dot(ba,rd);
    float baoc = dot(ba,oc);
    float k2 = baba            - bard*bard;
    float k1 = baba*dot(oc,rd) - baoc*bard;
    float k0 = baba*dot(oc,oc) - baoc*baoc - ra*ra*baba;
    float h = k1*k1 - k2*k0;
    if( h<0.0 ) return vec4(-1.0);//no intersection
    h = sqrt(h);
    float t = (-k1-h)/k2;
    // body
    float y = baoc + t*bard;
    if( y>0.0 && y<baba ) return vec4( t, (oc+t*rd - ba*y/baba)/ra );
    // caps
    t = ( ((y<0.0) ? 0.0 : baba) - baoc)/bard;
    if( abs(k1+k2*t)<h )
    {
        return vec4( t, ba*sign(y)/sqrt(baba) );
    }
    return vec4(-1.0);//no intersection
}



vec4 CastRay( inout vec3 ro, inout vec3 rd, vec3 light_pos ) {
    
    vec4 col; //color vector 

    vec2 inter = vec2(MAX_DIST);

    vec2 temp_inter = vec2( 0.0 );
        
    vec3 n = vec3( 0.0 );

    //TESTING ROOM


    vec3 sphere_pos = vec3( 0.0 );

    for ( int i = 0; i < DEFAULT_SIZE; i++ ) {

        if ( spheres_pos[i].w < eps ) continue;

        sphere_pos = (spheres_pos[i]).xyz;
        temp_inter = SphIntersect( ro + sphere_pos, rd, (spheres_pos[i]).w );

        if ( ( temp_inter.x > 0.0) && ( temp_inter.x < inter.x ) ) {
        
            inter = temp_inter;
            vec3 inter_pos = ro + rd * inter.x;
            n = normalize(inter_pos + sphere_pos);
            col = spheres_col[i];

        }

    }
    
    vec3 box_norm = vec3( 0.0 );
    vec3 box_size = vec3( 0.0 );
    vec3 box_pos = vec3( 0.0 );

    for ( int i = 0; i < DEFAULT_SIZE; i++ ) {

        if ( boxes_size[i].x < eps && boxes_size[i].y < eps && boxes_size[i].z < eps ) continue;

        box_pos = boxes_pos[i];
        box_size = boxes_size[i];

        temp_inter = boxIntersection( ro + box_pos , rd, box_size, box_norm);

        if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

            inter = temp_inter;
            n = box_norm;
            col = boxes_col[i];

        }

    }

    vec3 pa = vec3( 0.0 );
    vec3 pb = vec3( 0.0 );
    float ra = 0.0;
    float rb = 0.0;

    for ( int i = 0; i < DEFAULT_SIZE; i++ ) {

        if ( cones_up_point[i].w < eps && cones_down_point[i].w < eps ) continue;

        pa = cones_down_point[i].xyz;
        pb = cones_up_point[i].xyz;
        ra = cones_down_point[i].w;
        rb = cones_up_point[i].w;

        temp_inter = vec2(coneIntersect( ro, rd, pa, pb, ra, rb));

        if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

            inter = temp_inter;
            n = coneIntersect( ro, rd, pa, pb, ra, rb).yzw;
            col = cones_col[i];

        }

    }

    vec3 a = vec3( 0.0 );
    vec3 b = vec3( 0.0 );
    float r = 0.0;

    for ( int i = 0; i < DEFAULT_SIZE; i++ ) {

        if ( cyl_up_point[i].w < eps ) continue;

        a = cyl_up_point[i].xyz;
        b = cyl_down_point[i];
        r = cyl_up_point[i].w;

        temp_inter = vec2( cylIntersect( ro, rd, a, b, r) );

        if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

            inter = temp_inter;
            n = normalize( cylIntersect( ro, rd, a, b, r ).yzw );
            col = cyl_col[i];

        }

    }

    for ( int i = 0; i < PLANES_SIZE; i++ ) {

        if ( planes_norm[i].x < eps &&  planes_norm[i].y < eps &&  planes_norm[i].z < eps )

        temp_inter = vec2( plaIntersect( ro , rd, vec4( planes_norm[i], 1.0 ) ) );

        if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

            inter = temp_inter;
            n = planes_norm[i];
            col = planes_col[i];
            
        }

    }

    if( inter.x == MAX_DIST ) 
        return vec4( GetSky( rd, light_pos ), -2.0 ); // no intesepction, no ray 
    
    if ( col.a == -2.0 ) return col;  // it's a light source 

    vec3 spec = reflect( rd, n );

    float ref_p_pol = 1.0;
    float tr_p_pol = 0.0;
    float eta = 0.0;

    //please change refraction

    if ( col.a < 0.0 ) {

        float fresnel = 1.0 - abs ( dot ( -rd, n ) );

        if ( random() - 0.1 < fresnel * fresnel ) {

            rd = spec;
            return col;

        }

        ro += rd * ( inter.y + 0.001 );
        rd = refract( rd, n, 1.0 / (1.0 - col.a) );
        return col;

    }
    
    vec3 rand = randomOnSphere();
    vec3 diff = normalize ( rand * dot( rand, n ) );
    ro += rd * (inter.x - 0.001 );
    rd = mix ( spec, diff, col.a );
    return col;

}

vec3 TraceRay ( vec3 ro, vec3 rd, vec3 light_pos ) {

    vec3 col = vec3( 1.0 );
    vec4 reflection;
    int ref_num = 16;
    for ( int i = 0; i < ref_num; i++ ) {    

        reflection = CastRay( ro, rd, light_pos);
        col *= reflection.rgb;

        if ( reflection.a == -2.0 )
            return col;
            
        
    }

    return vec3( 0.0 );

}

void main() {   

    vec2 camera_place = vec2(0.5, 0.25);
    vec2 uv = ( gl_TexCoord[0].xy - camera_place ) * u_resolution / u_resolution.y;
    vec2 uvRes = hash22(uv + 1.0) * u_resolution + u_resolution;

    R_STATE.x = uint(u_seed1.x + uvRes.x);
	R_STATE.y = uint(u_seed1.y + uvRes.x);
	R_STATE.z = uint(u_seed2.x + uvRes.y);
	R_STATE.w = uint(u_seed2.y + uvRes.y);

    vec3 ray_origin = u_camera_pos; //setting ray start
    vec3 ray_direction = normalize( vec3(1.0, uv) ); //setting ray dir
    ray_direction.zx *= rot(-u_mouse.y);
    ray_direction.xy *= rot(u_mouse.x);     
    vec3 light_pos = normalize( ulight_pos );
    vec3 color = vec3( 0.0 );
    int samples = 4;

    for ( int i = 0; i < samples; i++ )
        color +=  TraceRay( ray_origin, ray_direction, light_pos);
    
    // making color real
    
    color /= samples ;

    float white = 20.0;
    float exposure = 16.0;

    color *= white * exposure;
    color = (color * (1.0 + color / white / white )) / ( 1.0 + color );
    vec3 sample_col = texture( u_sample, gl_TexCoord[0].xy).rgb;
    color = mix( sample_col, color, u_sample_part);

    gl_FragColor = vec4( color, 1.0 );

}

