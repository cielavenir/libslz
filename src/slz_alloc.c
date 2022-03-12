#include "slz.h"
#include <stdlib.h>

struct slz_stream *slz_alloc(){
return (struct stream*)malloc(sizeof(struct slz_stream));
}
void slz_free(struct slz_stream *s){
free(s);
}
