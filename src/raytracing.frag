#version 140

uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_mouse;
uniform vec3 u_camera_pos;
uniform vec2 u_seed1;
uniform vec2 u_seed2;
uniform sampler2D u_sample;
uniform float u_sample_part;

uniform float sun_brightness;

uniform vec4 spheres_pos[20];
uniform vec4 spheres_col[20];

uniform vec3 boxes_pos[20];
uniform vec3 boxes_size[20];
uniform vec4 boxes_col[20];

uniform vec3 planes_norm[20];
uniform vec4 planes_col[20];

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



vec4 CastRay( inout vec3 ro, inout vec3 rd, vec3 light_pos ) {
    
    vec4 col; //color vector 

    vec2 inter = vec2(MAX_DIST);

    vec2 temp_inter = vec2( 0.0 );
        
    vec3 n = vec3( 0.0 );

    //TESTING ROOM


    vec3 sphere_pos = vec3( 0.0 );

    int spheres_num = 20;

    for ( int i = 0; i < spheres_num; i++ ) {

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

    for ( int i = 0; i < 20; i++ ) {

        box_pos = boxes_pos[i];
        box_size = boxes_size[i];
    
        temp_inter = boxIntersection( ro + box_pos , rd, box_size, box_norm);

        if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

            inter = temp_inter;
            n = box_norm;
            col = boxes_col[i];

        }

    }

    for ( int i = 0; i < 20; i++ ) {

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
    vec3 light_pos = normalize( vec3( 0.5 , -0.75, 0.8 ) );
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

