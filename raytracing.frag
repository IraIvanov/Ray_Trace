#version 130


uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_mouse;
uniform vec3 u_camera_pos;
uniform vec2 u_seed1;
uniform vec2 u_seed2;

const float MAX_DIST = 99999.0; 
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
    return clamp( sun + sky_col, 0.0 , 1.0  );

}

/*vec3 getSky(vec3 rd) {
	vec3 col = vec3(0.3, 0.6, 1.0);
	vec3 sun = vec3(0.95, 0.9, 1.0);
	sun *= max(0.0, pow(dot(rd, light), 256.0));
	col *= max(0.0, dot(light, vec3(0.0, 0.0, -1.0)));
	return clamp(sun + col * 0.01, 0.0, 1.0);
}*/


vec2 SphIntersect( in vec3 ro, in vec3 rd/*, in vec3 ce*/, float ra ) {   // func that seek ray intersection
    
    vec3 oc = ro /*- ce*/;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - ra*ra;
    float h = b*b - c;
    
    if( h < 0.0 )
        return vec2( -1.0 ); // no intersection
    
    h = sqrt( h );
    
    return vec2( -b - h, -b + h );
}

vec2 boxIntersection( in vec3 ro, in vec3 rd, in vec3 boxSize, out vec3 outNormal ) {
    
    vec3 m = 1.0/rd; // can precompute if traversing a set of aligned boxes
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

    vec3 sphere_pos = vec3( 0.0, 0.0, 0.0 );
    vec2 inter = vec2(MAX_DIST);

    vec2 temp_inter = SphIntersect( ro + sphere_pos, rd, 1.0 );

    //temporary please change objecct placing
    
    vec3 n;

    if ( ( temp_inter.x > 0.0) && ( temp_inter.x < inter.x ) ) {
     
        inter = temp_inter;
        vec3 inter_pos = ro + rd * inter.x; // coords of intersection point
        n = normalize(inter_pos - sphere_pos); // normal vector
        col = vec4( 1.0, 0.2, 0.1, 0.005 ); //red

    }

    //TESTING ROOM

    vec3 sphere_pos2 = vec3( 4.0, 0.0, 0.0 );
    temp_inter = SphIntersect( ro + sphere_pos2, rd, 1.0 );

    if ( ( temp_inter.x > 0.0) && ( temp_inter.x < inter.x ) ) {
     
        inter = temp_inter;
        vec3 inter_pos = ro + rd * inter.x; // coords of intersection point
        n = normalize(inter_pos + sphere_pos2); // normal vector
        col = vec4( 1.0, 1.0, 1.0, -1.333 ); //blue

    }

    vec3 sphere_pos3 = vec3( -4.0, 0.0, 0.0 );
    temp_inter = SphIntersect( ro + sphere_pos3, rd, 1.0 );

    if ( ( temp_inter.x > 0.0) && ( temp_inter.x < inter.x ) ) {
     
        inter = temp_inter;
        vec3 inter_pos = ro + rd * inter.x; // coords of intersection point
        n = normalize(inter_pos + sphere_pos3); // normal vector
        col = vec4( vec3(1.0), 0.0 ); //blue

    }

    //TESTING ROOM

    vec3 box_norm;
    vec3 boxSize = vec3( 1.0 );
    vec3 box_pos = vec3( 0.0, -4.0, 0.0 );
    temp_inter = boxIntersection( ro + box_pos , rd, boxSize, box_norm);

    if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

        inter = temp_inter;
        n = box_norm;
        col = vec4( 0.4, 0.6, 0.8, 0.5); //blue

    }

    vec3 box_norm2;
    vec3 boxSize2 = vec3( 1.0 );
    vec3 box_pos2 = vec3( 0.0, -10.0, 0.0 );
    temp_inter = boxIntersection( ro + box_pos2 , rd, boxSize2, box_norm2);

    if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

        inter = temp_inter;
        n = box_norm2;
        col = vec4( 0.6, 0.4, 0.5, 0.8 ); //purple

    }

    vec3 plane_norm = vec3( 0.0, 0.0, -1.0 );

    temp_inter = vec2( plaIntersect( ro , rd, vec4( plane_norm, 1.0 ) ) );

    if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

        inter = temp_inter;
        n = plane_norm;
        col = vec4( 0.5 );
        
    }
    if( inter.x == MAX_DIST )
        return vec4( -2.0 ); // no intesepction, no ray 
    
    if ( col.a == -2.0 ) return col;

    vec3 spec = reflect( rd, n );

    if ( col.a < 0.0 ) {

        if ( atan ( 1.0, 1.0 - col.a   ) >=  abs ( dot ( n, rd ) ) ) {

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
    for ( int i = 0; i < 8; i++ ) {    

        reflection = CastRay( ro, rd, light_pos);
        if ( reflection.a == -2.0 ) return col * GetSky( rd, light_pos );
            col *= reflection.rgb;
        
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
    vec3 light_pos = normalize( vec3( 0.5 , -0.75, 0.5 ) );
    //vec3 light_pos = normalize( vec3( cos( u_time ), -0.75, sin( u_time ) ) );
    vec3 color = TraceRay( ray_origin, ray_direction, light_pos);
    color.rgb = vec3( pow( color.r, 0.45), pow( color.g, 0.45), pow( color.b, 0.45) );

    gl_FragColor = vec4( color, 1.0 );

}

