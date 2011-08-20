/****************************************************************************/
/*                                                                          */
/*  Module:         jamexp.h                                                */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Prototypes for expression evaluation functions          */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMEXP_H
#define INC_JAMEXP_H

#include <stdint.h>

JAM_RETURN_TYPE urj_jam_evaluate_expression
    (char *expression, int32_t *result, JAME_EXPRESSION_TYPE *result_type);

#endif /* INC_JAMEXP_H */
