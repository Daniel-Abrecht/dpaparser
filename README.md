# What is dpaparser?

dpaparser is a parser generater, currently only for parsing yaml files.
Templates, which are valid c source and header files, define a macro
specifying all structures & fields to be parsed, and include the parser
generator, which generates a valid c header if it is included as normal
in a c source or header, and generates a valid c source if compiled
using the dpaparser script.

The parser is generated by the dpaparser script which is implemented as
an executable  make script. More specifically, it's a script which uses
make as it's interpreter, and is a normal make file otherwise.
It uses a c compiler to compile the parser code.


## WTF, why?

I quickly needed a high level c yaml parser and didn't want to have to
deal with writing and messing with yaml parser code to fill structures
using a lower level yaml parser all the time. This was originally included
in another project, but I wanted to use it in other projects too, which
is why this exists and why this is solved in such an unusual way.


# How do I use it?
## Building & installing

Bulding the libdpaparser.a library:
```
make
```

Installing the library, include files, and dpaparser script:
```
sudo make PREFIX=/usr/local install
```
PREFIX specifies the install location. /usr/local is the default.

Uninstalling it:
```
sudo make PREFIX=/usr/local uninstall
```


## Using dpaparser

Make sure all your templates are in some folder and have the .template extension.
You can include these templates into your c program and they'll be valid c headers.
To generate the parser code, use the following command:
```
dpaparser TEMPLATE=my_template_dir/ TEMPLATE_A=lib/libmytemplateparsers.a
```
This will generate & compile the parser code for all templates in the template directory
specified by `TEMPLATE`, and generate a static library at wherever `TEMPLATE_A` specifies
containing them. It will temporarely store the objectfiles at `BUILD=build` and uses the
compiler specified using `CC`. To use these libraries, you need to link against it,
the dpaparser static library, and if you use the yaml parser generater, against the
yaml library too.

Use the following macros for parsing:
```
PARSE_YAML( TYPE, FILE, POINTER_TO_STRUCTURE_POINTER );
FREE_YAML( TYPE, POINTER_TO_STRUCTURE_POINTER );
```
An example how to use these macros, assuming the template further below:
```
  FILE *file = fopen("example.yaml", "r");
  if(!file){
    fprintf(stderr,"Failed to open file!\n");
    exit(1);
  }

  gen_my_structure_t* result;
  if( !PARSE_YAML(unitscript,file,&result) || !result ){
    fprintf(stderr,"something went wrong!\n");
    fclose(file);
    exit(1);
  }
  fclose(file);

  if(result->some_integer_property){
    printf("'a' was set to %ld\n", *result->some_integer_property);
  }else{
    printf("'a' not found");
  }

  FREE_YAML( my_structure, &result );
```


## Templete example & details

```
// Include guard
#ifndef THE_INCLUDE_GUARD
#define THE_INCLUDE_GUARD

// Include any code you want and is suitable to a c header

...

// The templates
#define PARSABLE_STRUCTURES \
  BLOCK( my_structure, ( \
    ENTRY( string, "the yaml property", the_string_property ) \
    ENTRY( integer, "a", some_integer_property ) \
    ENTRY( boolean, "b", some_boolean_property ) \
    ENTRY( map, "c", some_map ) \
    ENTRY( list, "d", some_list ) \
    PRIVATE( \
      size_t some_other_properties_not_parsed; \
      size_t some_other_properties_not_parsed2; \
    ) \
  ))

// This header generates the c structures, declarations, functions, yaml parsercode, etc.
#include <dpaparser/yaml>
#endif
```

The `ENTRY` and `PRIVATE` macros are optional. The types specified in `ENTRY` and `BLOCK`
correspond to the `gen_ ## typename ## _t` and `struct gen_ ## typename` type.
The type of an `ENTRY` can be one defined by a `BLOCK`.

Each entry is a struct field, and it's always a pointer which is null if tha yaml file
didn't include it.

Using other types than strings as values of maps and lists is currently not supported.
