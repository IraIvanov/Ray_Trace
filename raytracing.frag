uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_mouse;
uniform vec3 u_camera_pos;

const float MAX_DIST = 99999.0;

mat2 rot( float a ) {

    return mat2( cos(a), -sin(a), sin(a), cos(a) );

}


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

vec2 boxIntersection( in vec3 ro, in vec3 rd, vec3 boxSize, out vec3 outNormal ) {
    
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
    return -(dot(ro,p.xyz)+p.w)/dot(rd,p.xyz);
}



vec3 CastRay( vec3 ro, vec3 rd, vec3 light_pos ) {
    
    vec3 sphere_pos = vec3( 0.0, 0.0, 0.0 );
    vec2 inter = vec2(MAX_DIST);

    vec2 temp_inter = SphIntersect( ro + sphere_pos, rd, 1.0 );

    //temporary please change objecct placing
    vec3 n;

    if ( ( temp_inter.x > 0.0) && ( temp_inter.x < inter.x ) ) {
     
        inter = temp_inter;
        vec3 inter_pos = ro + rd * inter.x; // coords of intersection point
        n = inter_pos - sphere_pos; // normal vector
        
    }

    vec3 box_norm;
    vec3 boxSize = vec3( 1.0 );
    vec3 box_pos = vec3( 0.0, -2.0, 0.0 );
    temp_inter = boxIntersection( ro + box_pos , rd, boxSize, box_norm);

    if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

        inter = temp_inter;
        n = box_norm;
        
    }

    vec3 plane_norm = vec3( 0.0, 0.0, -1.0 );

    temp_inter.x = plaIntersect( ro , rd, vec4( plane_norm, 1.0 ) );
    temp_inter.y = plaIntersect( ro , rd, vec4( plane_norm, 1.0 ) );

    if ( (temp_inter.x > 0.0) && (temp_inter.x < inter.x) ) {

        inter = temp_inter;
        n = plane_norm;
        
    }

    if( inter.x == MAX_DIST )
        return vec3( 0.0 ); // no intesepction, no ray 


    float light_power = 0.5; // brightness coef
    float reflection_coef = 32.0;
    float minimal_brightness = 0.5; //speaks for itself 
    float lighting = max( 0.0, dot( n, -light_pos )) * light_power + minimal_brightness ; 
    vec3 reflected = reflect( rd, n );
    float specular = pow ( max( 0.0, dot(reflected, -light_pos) ), reflection_coef ) ;
    vec3 result = vec3(1.0, 0.6, 0.75); //put colour here, basic colour is white 

    return result.xyz * max ( 0.0, lighting + specular );//vec3( max( 0.0, lighting + specular ) );

}


void main() {

    vec2 camera_place = vec2(0.5, 0.25);
    vec2 uv = ( gl_TexCoord[0].xy - camera_place ) * u_resolution / u_resolution.y;
    vec3 ray_origin = u_camera_pos; //setting ray start
    vec3 ray_direction = normalize( vec3(1.0, uv) ); //setting ray dir
    ray_direction.zx *= rot(-u_mouse.y);
    ray_direction.xy *= rot(u_mouse.x);     
    vec3 light_pos = normalize( vec3( cos( u_time ), -0.75, sin( u_time ) ) );  // normalize( vec3( cos( u_time ), -0.75, sin( u_time ) ) ); set beautyful lighting 
    vec3 color = CastRay( ray_origin, ray_direction, light_pos );
    gl_FragColor = vec4( color, 1.1 );

}

