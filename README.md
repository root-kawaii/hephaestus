# hephaestus

engine following vulkan-guide

TODO

//////////

need to be able to move objects that i put into the world
so i need to update logic position and render position
maybe do it with a nice interface

then keep overall fixing the code with the scene structure and the state

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
