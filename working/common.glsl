#ifdef VERT
#	define INPUT(type, name, index) layout(location = index) in type name
#	define OUTPUT(type, name, index)
#	define VARYING(type, name) out type name
#endif
#ifdef FRAG
#	define INPUT(type, name, index)
#	define OUTPUT(type, name, index) layout(location = index) out type name
#	define VARYING(type, name) in type name
#endif