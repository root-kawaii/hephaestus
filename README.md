# hephaestus

engine following vulkan-guide

TODO

//////////

need to be able to move objects that i put into the world
so i need to update logic position and render position
maybe do it with a nice interface

then keep overall fixing the code with the scene structure and the state

THEN SERIALIZATION and save scene and state

STILL dunno why camera as object works and not as pointer to object

///////
///////

//////
///////

implement rotation around object, best way is have a renderobject to which camera locks
and write function for rotation around renderobject position

8 directions movement ( ahead + side)

understand forward declaration

///////

physics ??

fix limited imgui input buffer

parse so it doesnt crash if there is wrong inputs

implemet colors in konsole

implement cd and folder navigation (keep a path variable in the engine ? or reset everytime ? probably reset everytime but starting dir has to be the general one (not bin or src))

improve parsing probably
autocompletion of engine konsole
new commands ?
understand shaders

# TODO

vec3 rayCast(double xpos, double ypos, mat4 projection, mat4 view) {
// converts a position from the 2d xpos, ypos to a normalized 3d direction
float x = (2.0f _ xpos) / WIDTH - 1.0f;
float y = 1.0f - (2.0f _ ypos) / HEIGHT;
float z = 1.0f;
vec3 ray*nds = vec3(x, y, z);
vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
// eye space to clip we would multiply by projection so
// clip space to eye space is the inverse projection
vec4 ray_eye = inverse(projection) * ray*clip;
// convert point to forwards
ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
// world space to eye space is usually multiply by view so
// eye space to world space is inverse view
vec4 inv_ray_wor = (inverse(view) * ray_eye);
vec3 ray_wor = vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
ray_wor = normalize(ray_wor);
return ray_wor;
}

https://stackoverflow.com/questions/70239556/for-mouse-click-ray-casting-a-line-why-arent-my-starting-rays-updating-to-my-c
