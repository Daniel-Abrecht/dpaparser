// Copyright (c) 2019 Daniel Abrecht
// SPDX-License-Identifier: WTFPL OR MIT

#define GEN_YAML_PARSER_INTERNALS

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <dpaparser/yaml>

void free_yaml_integer( gen_integer_t** ch ){
  if(!*ch) return;
  free(*ch);
  *ch = 0;
}

void free_yaml_boolean( gen_boolean_t** ch ){
  if(!*ch) return;
  free(*ch);
  *ch = 0;
}

bool parse_yaml_boolean( struct gen_parser* s, gen_boolean_t** ch ){
  gen_boolean_t* c = malloc(sizeof(gen_integer_t));
  if(!c){
    perror("malloc failed");
    s->done = true;
    return false;
  }
  if( !strcmp(s->value,"yes") ){
    *c = true;
  }else if( !strcmp(s->value,"no") ){
    *c = false;
  }else{
    fprintf(stderr,"Expected 'yes' or 'no'\n");
    s->done = true;
    free(c);
    return false;
  }
  *ch = c;
  return true;
}

bool parse_integer( size_t length, const char value[length], gen_integer_t* c ){
  char* end = 0;
  long l = strtol(value, &end, 0);
  if( !length || (size_t)(end-value) != length ){
    fprintf(stderr,"Couldn't parse number: %.*s\n",(int)length,value);
    return false;
  }
  if( ( l == LONG_MIN || l == LONG_MAX ) && errno == ERANGE ){
    fprintf(stderr,"Number out of range: %.*s\n",(int)length,value);
    return false;
  }
  *c = l;
  return true;
}

bool parse_yaml_integer( struct gen_parser* s, gen_integer_t** ch ){
  gen_integer_t* c = malloc(sizeof(gen_integer_t));
  if(!c){
    perror("malloc failed");
    s->done = true;
    return false;
  }
  if( !parse_integer(s->length,s->value,c) ){
    free(c);
    s->done = true;
    return false;
  }
  *ch = c;
  return true;
}

void free_yaml_string( gen_string_t** ch ){
  gen_string_t* c = *ch;
  if(!c) return;
  *ch = 0;
  if( c->data )
    free(c->data);
  free(c);
}

void free_yaml_map( gen_map_t** ch ){
  gen_map_t* c = *ch;
  if(!c) return;
  *ch = 0;
  while( c->length-- ){
    gen_map_entry_t* e = c->entries + c->length;
    if( e->key.data )
      free(e->key.data);
    if( e->value.data )
      free(e->value.data);
  }
  if( c->entries )
    free(c->entries);
  free(c);
}

void free_yaml_list( gen_list_t** ch ){
  gen_list_t* c = *ch;
  if(!c) return;
  *ch = 0;
  while(c->length--)
    free(c->entries[c->length].data);
  if( c->entries )
    free(c->entries);
  free(c);
}

bool parse_yaml_string( gen_parser_t* s, gen_string_t** ch ){
  if( s->state != PARSER_STATE_VALUE )
    return true;
  gen_string_t* c = calloc(1,sizeof(gen_string_t));
  if(!c){
    perror("calloc failed");
    s->done = true;
    return false;
  }
  *ch = c;
  c->data = malloc(s->length+1);
  memcpy(c->data,s->value,s->length);
  c->data[s->length] = 0;
  c->length = s->length;
  return true;
}

bool parse_yaml_map( struct gen_parser* s, gen_map_t** ch ){
  yaml_token_t token;

  if( s->state != PARSER_STATE_MAPPING )
    return false;

  size_t length = 0;
  char* key = 0;
  bool done = false;

  gen_map_t* c = calloc(1,sizeof(gen_map_t));
  if(!c){
    perror("calloc failed");
    s->done = true;
    return false;
  }
  *ch = c;

  bool empty = true;

  do {

    if( !yaml_parser_scan(s->parser, &token) ){
      fprintf(stderr,"yaml_parser_scan failed\n");
      s->done = true;
      return false;
    }

    bool next = false;

    switch( token.type ){
      case YAML_KEY_TOKEN: {
        s->state = PARSER_STATE_KEY;
      } break;
      case YAML_VALUE_TOKEN: {
        s->state = PARSER_STATE_VALUE;
      } break;
      case YAML_SCALAR_TOKEN: {
        if( s->state == PARSER_STATE_KEY ){
          length = token.data.scalar.length;
          if(key){
            if(!empty){
              fprintf(stderr,"Warning: failed to process key \"%s\". Wrong value type?\n", key);
              free(key);
              key = 0;
            }else{
              gen_map_entry_t* el = realloc( c->entries, (c->length+1) * sizeof(gen_map_entry_t) );
              if(!el){
                perror("realloc failed");
                s->done = true;
                return false;
              }
              c->entries = el;
              gen_map_entry_t* e = el + c->length;
              e->key.data = key;
              e->key.length = length;
              e->value.data = 0;
              e->value.length = 0;
              c->length++;
              key = 0;
            }
          }
          key = malloc(length+1);
          if(!key){
            perror("malloc failed");
            s->done = true;
            return false;
          }
          memcpy(key,token.data.scalar.value,length);
          key[length] = 0;
          empty = true;
        }else{
          empty = false;
          s->value = (const char*)token.data.scalar.value;
          s->length = token.data.scalar.length;
          next = true;
        }
      } break;
      case YAML_FLOW_MAPPING_START_TOKEN:
      case YAML_BLOCK_MAPPING_START_TOKEN: {
        fprintf(stderr,"Expected <key>:<value> list\n");
        s->done = true;
        if(key){
          free(key);
          key = 0;
        }
        return false;
      } break;
      case YAML_BLOCK_END_TOKEN: {
        done = true;
      } break;
      default: empty = false; break;
    }

    if( next ){
      gen_map_entry_t* el = realloc( c->entries, (c->length+1) * sizeof(gen_map_entry_t) );
      if(!el){
        perror("realloc failed");
        s->done = true;
        return false;
      }
      c->entries = el;
      gen_map_entry_t* e = el + c->length;
      e->key.data = key;
      e->key.length = length;
      e->value.length = 0;
      c->length++;
      e->value.data = malloc(s->length+1);
      if(!e->value.data){
        perror("malloc failed");
        s->done = true;
        return false;
      }
      memcpy(e->value.data,s->value,s->length);
      e->value.data[s->length] = 0;
      e->value.length = s->length;
      key = 0;
    }

    s->done = token.type == YAML_STREAM_END_TOKEN;
    yaml_token_delete(&token);
  } while( !s->done && !done );

  if(key){
    free( key );
    key = 0;
  }

  return true;
}

bool parse_yaml_list( struct gen_parser* s, gen_list_t** ch ){
  yaml_token_t token;

  if( s->state != PARSER_STATE_SEQUENCE )
    return false;

  bool done = false;

  gen_list_t* c = calloc(1,sizeof(gen_list_t));
  if(!c){
    perror("calloc failed");
    s->done = true;
    return false;
  }
  *ch = c;

  do {

    if( !yaml_parser_scan(s->parser, &token) ){
      fprintf(stderr,"yaml_parser_scan failed\n");
      s->done = true;
      return false;
    }

    bool next = false;

    switch( token.type ){
      case YAML_KEY_TOKEN: {
        fprintf(stderr,"Unexpected token YAML_KEY_TOKEN\n");
        s->done = true;
        return false;
      } break;
      case YAML_VALUE_TOKEN: {
        s->state = PARSER_STATE_VALUE;
      } break;
      case YAML_SCALAR_TOKEN: {
        s->value = (const char*)token.data.scalar.value;
        s->length = token.data.scalar.length;
        next = true;
      } break;
      case YAML_BLOCK_MAPPING_START_TOKEN: {
        fprintf(stderr,"Expected - <key> list\n");
        s->done = true;
        return false;
      } break;
      case YAML_BLOCK_END_TOKEN: {
        done = true;
      } break;
      default: break;
    }

    if( next ){
      gen_string_t* el = realloc( c->entries, (c->length+1) * sizeof(gen_string_t) );
      if(!el){
        perror("realloc failed");
        s->done = true;
        return false;
      }
      c->entries = el;
      gen_string_t* e = el + c->length;
      e->length = 0;
      c->length++;
      e->data = malloc(s->length+1);
      if(!e->data){
        perror("malloc failed");
        s->done = true;
        return false;
      }
      memcpy(e->data,s->value,s->length);
      e->data[s->length] = 0;
      e->length = s->length;
    }

    s->done = token.type == YAML_STREAM_END_TOKEN;
    yaml_token_delete(&token);
  } while( !s->done && !done );

  return true;
}

bool parse_yaml_skip_unknown_mapping( struct gen_parser* s ){
  size_t count = 1;
  do {
    yaml_token_t token;
    if( !yaml_parser_scan(s->parser, &token) ){
      fprintf(stderr,"yaml_parser_scan failed\n");
      s->done = true;
      return false;
    }
    if( token.type == YAML_BLOCK_MAPPING_START_TOKEN ){
      count++;
    }else if( token.type == YAML_BLOCK_END_TOKEN ){
      count--;
    }else if( token.type == YAML_STREAM_END_TOKEN ){
      s->done = true;
    }
    yaml_token_delete(&token);
  } while( !s->done && count );
  return true;
}

bool parse_yaml(FILE* file, void** ret, bool(*parser_func)(gen_parser_t* s, void** ret), void(*free_func)(void** ret) ){
  yaml_parser_t parser;

  *ret = 0;

  if(!yaml_parser_initialize(&parser)){
    fprintf(stderr,"Failed to initialize parser!\n");
    return false;
  }

  yaml_parser_set_input_file(&parser, file);

  gen_parser_t s = {
    .parser = &parser,
    .value = 0,
    .length = 0,
    .state = PARSER_STATE_MAPPING,
    .done = false
  };

  bool done = false;
  bool found = false;
  do {
    yaml_token_t token;
    if( !yaml_parser_scan(&parser, &token) ){
      fprintf(stderr,"yaml_parser_scan failed\n");
      goto failed_after_parser_initialize;
    }
    if( token.type == YAML_BLOCK_MAPPING_START_TOKEN )
      found = true;
    if( token.type == YAML_STREAM_END_TOKEN )
      done = true;
    yaml_token_delete(&token);
  } while( !done && !found );
  if(!found){
    fprintf(stderr,"No datas found\n");
    goto failed_after_parser_initialize;
  }

  if( !(*parser_func)( &s, ret ) )
    goto failed_after_parser_initialize;

  yaml_parser_delete(&parser);
  return true;
failed_after_parser_initialize:
  yaml_parser_delete(&parser);
  if(*ret)
    (*free_func)(ret);
  return false;
}
