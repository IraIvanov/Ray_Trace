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

const float MAX_DIST = 99999.0;
const float eps = 0.0001; 

mat2 rot( float a ) {

    return mat2( cos(a), -sin(a), sin(a), cos(a) );

}

vec3 GetSky( vec3 rd, vec3 light_pos ) {

    vec3 sky_col = vec3( 0.3, 0.6, 1.0 ); //blue skyes
    vec3 sun = vec3( 0.95, 0.9, 1.0 ); // sun
    sun *= pow ( max( 0.0, dot(rd, -light_pos) ), 32.0 );
    return clamp( sun + sky_col*sun_brightness, 0.0 , 1.0  );

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


vec4 coneIntersect( in vec3  ro, in vec3  rd, in vec3  pa, in vec3  pb, in float ra, in float rb )
{
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


vec3 CastRay( inout vec3 ro, inout vec3 rd, vec3 light_pos ) {

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
            n = normalize( coneIntersect( ro, rd, pa, pb, ra, rb).yzw);
            col = cones_col[i];

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
        return vec3( -1.0 ); // no intesepction, no ray 


    float light_power = 0.5; // brightness coef
    float reflection_coef = 32.0;
    float minimal_brightness = 0.5; //speaks for itself 
    float lighting = dot( n, -light_pos ) * light_power + minimal_brightness ; 
    vec3 reflected = reflect( rd, n );
    float specular = pow ( max( 0.0, dot(reflected, -light_pos) ), reflection_coef ) ;
    col.xyz *= mix(lighting, specular, 0.5 );
    ro += rd * (inter.x - 0.001 );
    rd = n;
    return col.xyz;

}

vec3 TraceRay ( vec3 ro, vec3 rd, vec3 light_pos ) {

    vec3 col = CastRay( ro, rd, light_pos );

        if ( col.x == -1.0 ) return GetSky( rd, light_pos );
    
        vec3 light = -light_pos;
        if ( dot(rd, light ) > 0.0 ) {
            if ( ((CastRay( ro, light, light_pos )).x) != -1.0 ) col *= 0.5;
        }
    return col;

}

void main() {   

    vec2 camera_place = vec2(0.5, 0.25);
    vec2 uv = ( gl_TexCoord[0].xy - camera_place ) * u_resolution / u_resolution.y;
    vec3 ray_origin = u_camera_pos; //setting ray start
    vec3 ray_direction = normalize( vec3(1.0, uv) ); //setting ray dir
    ray_direction.zx *= rot(-u_mouse.y);
    ray_direction.xy *= rot(u_mouse.x);     
    vec3 light_pos = normalize( ulight_pos );
    vec3 color = TraceRay( ray_origin, ray_direction, light_pos);
    color.rgb = vec3( pow( color.r, 0.45), pow( color.g, 0.45), pow( color.b, 0.45) );

    gl_FragColor = vec4( color, 1.0 );

}