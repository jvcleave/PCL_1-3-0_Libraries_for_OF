/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TerrainPolylineEditor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSmartPointer.h"
#include "vtkDEMReader.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkProjectedTerrainPath.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkWarpScalar.h"
#include "vtkPolyDataNormals.h"
#include "vtkLODActor.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkTerrainDataPointPlacer.h"
#include "vtkTerrainContourLineInterpolator.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkContourWidget.h"
#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkInteractorEventRecorder.h"
#include "vtkTestUtilities.h"

char TerrainPolylineEditorLog[] =
"# StreamVersion 1\n"
"EnterEvent 522 259 0 0 0 0 0 i\n"
"MouseMoveEvent 446 277 0 0 0 0 0 i\n"
"MouseMoveEvent 392 287 0 0 0 0 0 i\n"
"MouseMoveEvent 348 301 0 0 0 0 0 i\n"
"MouseMoveEvent 310 309 0 0 0 0 0 i\n"
"MouseMoveEvent 292 323 0 0 0 0 0 i\n"
"MouseMoveEvent 282 351 0 0 0 0 0 i\n"
"MouseMoveEvent 278 365 0 0 0 0 0 i\n"
"MouseMoveEvent 278 366 0 0 0 0 0 i\n"
"MouseMoveEvent 304 348 0 0 0 0 0 i\n"
"MouseMoveEvent 324 332 0 0 0 0 0 i\n"
"MouseMoveEvent 330 318 0 0 0 0 0 i\n"
"MouseMoveEvent 330 306 0 0 0 0 0 i\n"
"MouseMoveEvent 312 296 0 0 0 0 0 i\n"
"MouseMoveEvent 284 302 0 0 0 0 0 i\n"
"MouseMoveEvent 260 308 0 0 0 0 0 i\n"
"MouseMoveEvent 232 312 0 0 0 0 0 i\n"
"MouseMoveEvent 204 316 0 0 0 0 0 i\n"
"MouseMoveEvent 182 320 0 0 0 0 0 i\n"
"MouseMoveEvent 166 322 0 0 0 0 0 i\n"
"MouseMoveEvent 156 322 0 0 0 0 0 i\n"
"MouseMoveEvent 148 322 0 0 0 0 0 i\n"
"MouseMoveEvent 147 321 0 0 0 0 0 i\n"
"MouseMoveEvent 146 321 0 0 0 0 0 i\n"
"MouseMoveEvent 145 320 0 0 0 0 0 i\n"
"MouseMoveEvent 144 320 0 0 0 0 0 i\n"
"MouseMoveEvent 143 320 0 0 0 0 0 i\n"
"MouseMoveEvent 140 320 0 0 0 0 0 i\n"
"MouseMoveEvent 137 320 0 0 0 0 0 i\n"
"MouseMoveEvent 136 320 0 0 0 0 0 i\n"
"MouseMoveEvent 135 320 0 0 0 0 0 i\n"
"MouseMoveEvent 135 321 0 0 0 0 0 i\n"
"MouseMoveEvent 135 322 0 0 0 0 0 i\n"
"MouseMoveEvent 135 323 0 0 0 0 0 i\n"
"MouseMoveEvent 135 324 0 0 0 0 0 i\n"
"MouseMoveEvent 135 326 0 0 0 0 0 i\n"
"MouseMoveEvent 135 327 0 0 0 0 0 i\n"
"MouseMoveEvent 136 328 0 0 0 0 0 i\n"
"MouseMoveEvent 136 330 0 0 0 0 0 i\n"
"MouseMoveEvent 137 331 0 0 0 0 0 i\n"
"MouseMoveEvent 138 332 0 0 0 0 0 i\n"
"MouseMoveEvent 138 333 0 0 0 0 0 i\n"
"LeftButtonPressEvent 138 333 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 138 333 0 0 0 0 0 i\n"
"MouseMoveEvent 170 338 0 0 0 0 0 i\n"
"MouseMoveEvent 176 336 0 0 0 0 0 i\n"
"MouseMoveEvent 178 336 0 0 0 0 0 i\n"
"MouseMoveEvent 180 336 0 0 0 0 0 i\n"
"MouseMoveEvent 181 336 0 0 0 0 0 i\n"
"MouseMoveEvent 182 336 0 0 0 0 0 i\n"
"MouseMoveEvent 183 336 0 0 0 0 0 i\n"
"MouseMoveEvent 184 336 0 0 0 0 0 i\n"
"MouseMoveEvent 185 336 0 0 0 0 0 i\n"
"MouseMoveEvent 186 336 0 0 0 0 0 i\n"
"MouseMoveEvent 188 336 0 0 0 0 0 i\n"
"MouseMoveEvent 189 336 0 0 0 0 0 i\n"
"MouseMoveEvent 189 335 0 0 0 0 0 i\n"
"MouseMoveEvent 190 335 0 0 0 0 0 i\n"
"LeftButtonPressEvent 190 335 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 190 335 0 0 0 0 0 i\n"
"MouseMoveEvent 234 328 0 0 0 0 0 i\n"
"MouseMoveEvent 235 328 0 0 0 0 0 i\n"
"MouseMoveEvent 235 327 0 0 0 0 0 i\n"
"LeftButtonPressEvent 235 327 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 235 327 0 0 0 0 0 i\n"
"MouseMoveEvent 263 310 0 0 0 0 0 i\n"
"MouseMoveEvent 265 309 0 0 0 0 0 i\n"
"MouseMoveEvent 266 308 0 0 0 0 0 i\n"
"MouseMoveEvent 267 308 0 0 0 0 0 i\n"
"MouseMoveEvent 267 307 0 0 0 0 0 i\n"
"LeftButtonPressEvent 267 307 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 267 307 0 0 0 0 0 i\n"
"MouseMoveEvent 270 294 0 0 0 0 0 i\n"
"MouseMoveEvent 271 294 0 0 0 0 0 i\n"
"MouseMoveEvent 271 293 0 0 0 0 0 i\n"
"MouseMoveEvent 271 291 0 0 0 0 0 i\n"
"MouseMoveEvent 271 290 0 0 0 0 0 i\n"
"MouseMoveEvent 271 289 0 0 0 0 0 i\n"
"MouseMoveEvent 271 287 0 0 0 0 0 i\n"
"MouseMoveEvent 271 286 0 0 0 0 0 i\n"
"MouseMoveEvent 271 285 0 0 0 0 0 i\n"
"MouseMoveEvent 271 284 0 0 0 0 0 i\n"
"MouseMoveEvent 271 283 0 0 0 0 0 i\n"
"MouseMoveEvent 272 282 0 0 0 0 0 i\n"
"MouseMoveEvent 272 281 0 0 0 0 0 i\n"
"LeftButtonPressEvent 272 281 0 0 0 0 0 i\n"
"MouseMoveEvent 281 263 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 272 280 0 0 0 0 0 i\n"
"MouseMoveEvent 281 263 0 0 0 0 0 i\n"
"MouseMoveEvent 283 262 0 0 0 0 0 i\n"
"MouseMoveEvent 285 262 0 0 0 0 0 i\n"
"MouseMoveEvent 287 261 0 0 0 0 0 i\n"
"MouseMoveEvent 288 259 0 0 0 0 0 i\n"
"MouseMoveEvent 290 259 0 0 0 0 0 i\n"
"MouseMoveEvent 290 258 0 0 0 0 0 i\n"
"MouseMoveEvent 291 258 0 0 0 0 0 i\n"
"LeftButtonPressEvent 291 258 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 291 258 0 0 0 0 0 i\n"
"MouseMoveEvent 335 251 0 0 0 0 0 i\n"
"MouseMoveEvent 338 251 0 0 0 0 0 i\n"
"MouseMoveEvent 341 251 0 0 0 0 0 i\n"
"MouseMoveEvent 343 251 0 0 0 0 0 i\n"
"MouseMoveEvent 346 251 0 0 0 0 0 i\n"
"MouseMoveEvent 348 251 0 0 0 0 0 i\n"
"MouseMoveEvent 350 251 0 0 0 0 0 i\n"
"MouseMoveEvent 351 251 0 0 0 0 0 i\n"
"MouseMoveEvent 353 251 0 0 0 0 0 i\n"
"MouseMoveEvent 354 251 0 0 0 0 0 i\n"
"LeftButtonPressEvent 354 251 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 354 251 0 0 0 0 0 i\n"
"MouseMoveEvent 440 247 0 0 0 0 0 i\n"
"MouseMoveEvent 441 248 0 0 0 0 0 i\n"
"MouseMoveEvent 442 248 0 0 0 0 0 i\n"
"MouseMoveEvent 442 249 0 0 0 0 0 i\n"
"MouseMoveEvent 442 250 0 0 0 0 0 i\n"
"MouseMoveEvent 442 251 0 0 0 0 0 i\n"
"MouseMoveEvent 442 252 0 0 0 0 0 i\n"
"MouseMoveEvent 442 253 0 0 0 0 0 i\n"
"MouseMoveEvent 440 254 0 0 0 0 0 i\n"
"MouseMoveEvent 439 255 0 0 0 0 0 i\n"
"MouseMoveEvent 438 255 0 0 0 0 0 i\n"
"MouseMoveEvent 437 256 0 0 0 0 0 i\n"
"MouseMoveEvent 437 257 0 0 0 0 0 i\n"
"MouseMoveEvent 437 258 0 0 0 0 0 i\n"
"MouseMoveEvent 437 259 0 0 0 0 0 i\n"
"MouseMoveEvent 437 260 0 0 0 0 0 i\n"
"MouseMoveEvent 438 261 0 0 0 0 0 i\n"
"MouseMoveEvent 438 262 0 0 0 0 0 i\n"
"MouseMoveEvent 438 263 0 0 0 0 0 i\n"
"LeftButtonPressEvent 438 263 0 0 0 0 0 i\n"
"MouseMoveEvent 472 252 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 439 263 0 0 0 0 0 i\n"
"MouseMoveEvent 472 252 0 0 0 0 0 i\n"
"MouseMoveEvent 473 252 0 0 0 0 0 i\n"
"MouseMoveEvent 474 252 0 0 0 0 0 i\n"
"MouseMoveEvent 474 251 0 0 0 0 0 i\n"
"MouseMoveEvent 475 251 0 0 0 0 0 i\n"
"MouseMoveEvent 475 250 0 0 0 0 0 i\n"
"MouseMoveEvent 475 249 0 0 0 0 0 i\n"
"LeftButtonPressEvent 475 249 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 475 249 0 0 0 0 0 i\n"
"MouseMoveEvent 475 248 0 0 0 0 0 i\n"
"MouseMoveEvent 475 247 0 0 0 0 0 i\n"
"MouseMoveEvent 475 246 0 0 0 0 0 i\n"
"MouseMoveEvent 475 245 0 0 0 0 0 i\n"
"MouseMoveEvent 475 243 0 0 0 0 0 i\n"
"MouseMoveEvent 475 241 0 0 0 0 0 i\n"
"MouseMoveEvent 474 239 0 0 0 0 0 i\n"
"MouseMoveEvent 474 237 0 0 0 0 0 i\n"
"MouseMoveEvent 474 236 0 0 0 0 0 i\n"
"MouseMoveEvent 475 236 0 0 0 0 0 i\n"
"MouseMoveEvent 476 237 0 0 0 0 0 i\n"
"MouseMoveEvent 477 238 0 0 0 0 0 i\n"
"MouseMoveEvent 478 239 0 0 0 0 0 i\n"
"MouseMoveEvent 479 239 0 0 0 0 0 i\n"
"MouseMoveEvent 480 239 0 0 0 0 0 i\n"
"MouseMoveEvent 481 239 0 0 0 0 0 i\n"
"MouseMoveEvent 483 239 0 0 0 0 0 i\n"
"MouseMoveEvent 485 239 0 0 0 0 0 i\n"
"MouseMoveEvent 487 239 0 0 0 0 0 i\n"
"MouseMoveEvent 489 239 0 0 0 0 0 i\n"
"MouseMoveEvent 491 239 0 0 0 0 0 i\n"
"MouseMoveEvent 492 239 0 0 0 0 0 i\n"
"MouseMoveEvent 493 239 0 0 0 0 0 i\n"
"MouseMoveEvent 494 239 0 0 0 0 0 i\n"
"MouseMoveEvent 495 240 0 0 0 0 0 i\n"
"MouseMoveEvent 496 240 0 0 0 0 0 i\n"
"MouseMoveEvent 498 240 0 0 0 0 0 i\n"
"MouseMoveEvent 499 240 0 0 0 0 0 i\n"
"MouseMoveEvent 500 240 0 0 0 0 0 i\n"
"MouseMoveEvent 501 240 0 0 0 0 0 i\n"
"MouseMoveEvent 503 240 0 0 0 0 0 i\n"
"MouseMoveEvent 504 239 0 0 0 0 0 i\n"
"MouseMoveEvent 506 239 0 0 0 0 0 i\n"
"MouseMoveEvent 507 238 0 0 0 0 0 i\n"
"MouseMoveEvent 508 238 0 0 0 0 0 i\n"
"MouseMoveEvent 509 238 0 0 0 0 0 i\n"
"MouseMoveEvent 510 238 0 0 0 0 0 i\n"
"MouseMoveEvent 511 238 0 0 0 0 0 i\n"
"LeftButtonPressEvent 511 238 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 511 238 0 0 0 0 0 i\n"
"MouseMoveEvent 541 217 0 0 0 0 0 i\n"
"MouseMoveEvent 542 217 0 0 0 0 0 i\n"
"MouseMoveEvent 543 216 0 0 0 0 0 i\n"
"MouseMoveEvent 543 215 0 0 0 0 0 i\n"
"MouseMoveEvent 543 214 0 0 0 0 0 i\n"
"MouseMoveEvent 544 213 0 0 0 0 0 i\n"
"MouseMoveEvent 544 212 0 0 0 0 0 i\n"
"LeftButtonPressEvent 544 212 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 544 212 0 0 0 0 0 i\n"
"MouseMoveEvent 542 209 0 0 0 0 0 i\n"
"MouseMoveEvent 541 208 0 0 0 0 0 i\n"
"MouseMoveEvent 540 206 0 0 0 0 0 i\n"
"MouseMoveEvent 540 205 0 0 0 0 0 i\n"
"MouseMoveEvent 539 205 0 0 0 0 0 i\n"
"MouseMoveEvent 538 204 0 0 0 0 0 i\n"
"MouseMoveEvent 537 203 0 0 0 0 0 i\n"
"MouseMoveEvent 536 202 0 0 0 0 0 i\n"
"MouseMoveEvent 536 201 0 0 0 0 0 i\n"
"MouseMoveEvent 535 201 0 0 0 0 0 i\n"
"MouseMoveEvent 534 200 0 0 0 0 0 i\n"
"MouseMoveEvent 533 200 0 0 0 0 0 i\n"
"MouseMoveEvent 532 199 0 0 0 0 0 i\n"
"MouseMoveEvent 532 198 0 0 0 0 0 i\n"
"MouseMoveEvent 531 198 0 0 0 0 0 i\n"
"MouseMoveEvent 529 197 0 0 0 0 0 i\n"
"MouseMoveEvent 528 197 0 0 0 0 0 i\n"
"MouseMoveEvent 527 197 0 0 0 0 0 i\n"
"MouseMoveEvent 528 197 0 0 0 0 0 i\n"
"MouseMoveEvent 530 198 0 0 0 0 0 i\n"
"MouseMoveEvent 534 202 0 0 0 0 0 i\n"
"MouseMoveEvent 540 204 0 0 0 0 0 i\n"
"MouseMoveEvent 541 204 0 0 0 0 0 i\n"
"MouseMoveEvent 543 205 0 0 0 0 0 i\n"
"MouseMoveEvent 544 206 0 0 0 0 0 i\n"
"MouseMoveEvent 545 206 0 0 0 0 0 i\n"
"MouseMoveEvent 546 207 0 0 0 0 0 i\n"
"MouseMoveEvent 547 207 0 0 0 0 0 i\n"
"MouseMoveEvent 548 207 0 0 0 0 0 i\n"
"MouseMoveEvent 550 207 0 0 0 0 0 i\n"
"MouseMoveEvent 551 207 0 0 0 0 0 i\n"
"MouseMoveEvent 552 207 0 0 0 0 0 i\n"
"MouseMoveEvent 553 207 0 0 0 0 0 i\n"
"MouseMoveEvent 553 208 0 0 0 0 0 i\n"
"MouseMoveEvent 553 209 0 0 0 0 0 i\n"
"MouseMoveEvent 554 209 0 0 0 0 0 i\n"
"RightButtonPressEvent 554 209 0 0 0 0 0 i\n"
"RightButtonReleaseEvent 554 209 0 0 0 0 0 i\n"
"MouseMoveEvent 533 200 0 0 0 0 0 i\n"
"MouseMoveEvent 523 198 0 0 0 0 0 i\n"
"MouseMoveEvent 509 194 0 0 0 0 0 i\n"
"MouseMoveEvent 493 194 0 0 0 0 0 i\n"
"MouseMoveEvent 477 194 0 0 0 0 0 i\n"
"MouseMoveEvent 461 192 0 0 0 0 0 i\n"
"MouseMoveEvent 445 192 0 0 0 0 0 i\n"
"MouseMoveEvent 429 192 0 0 0 0 0 i\n"
"MouseMoveEvent 415 192 0 0 0 0 0 i\n"
"MouseMoveEvent 403 192 0 0 0 0 0 i\n"
"MouseMoveEvent 391 192 0 0 0 0 0 i\n"
"MouseMoveEvent 381 192 0 0 0 0 0 i\n"
"MouseMoveEvent 369 192 0 0 0 0 0 i\n"
"MouseMoveEvent 359 192 0 0 0 0 0 i\n"
"MouseMoveEvent 349 196 0 0 0 0 0 i\n"
"MouseMoveEvent 341 198 0 0 0 0 0 i\n"
"MouseMoveEvent 333 200 0 0 0 0 0 i\n"
"MouseMoveEvent 327 202 0 0 0 0 0 i\n"
"MouseMoveEvent 319 206 0 0 0 0 0 i\n"
"MouseMoveEvent 311 208 0 0 0 0 0 i\n"
"MouseMoveEvent 303 212 0 0 0 0 0 i\n"
"MouseMoveEvent 301 213 0 0 0 0 0 i\n"
"MouseMoveEvent 295 215 0 0 0 0 0 i\n"
"MouseMoveEvent 293 215 0 0 0 0 0 i\n"
"MouseMoveEvent 291 216 0 0 0 0 0 i\n"
"MouseMoveEvent 291 217 0 0 0 0 0 i\n"
"MouseMoveEvent 290 219 0 0 0 0 0 i\n"
"MouseMoveEvent 284 221 0 0 0 0 0 i\n"
"MouseMoveEvent 278 227 0 0 0 0 0 i\n"
"MouseMoveEvent 270 233 0 0 0 0 0 i\n"
"MouseMoveEvent 260 241 0 0 0 0 0 i\n"
"MouseMoveEvent 250 247 0 0 0 0 0 i\n"
"MouseMoveEvent 240 251 0 0 0 0 0 i\n"
"MouseMoveEvent 230 257 0 0 0 0 0 i\n"
"MouseMoveEvent 224 261 0 0 0 0 0 i\n"
"MouseMoveEvent 220 267 0 0 0 0 0 i\n"
"MouseMoveEvent 216 271 0 0 0 0 0 i\n"
"MouseMoveEvent 212 275 0 0 0 0 0 i\n"
"MouseMoveEvent 212 277 0 0 0 0 0 i\n"
"MouseMoveEvent 212 279 0 0 0 0 0 i\n"
"MouseMoveEvent 212 280 0 0 0 0 0 i\n"
"MouseMoveEvent 212 281 0 0 0 0 0 i\n"
"MouseMoveEvent 212 282 0 0 0 0 0 i\n"
"MouseMoveEvent 212 283 0 0 0 0 0 i\n"
"MouseMoveEvent 212 284 0 0 0 0 0 i\n"
"MouseMoveEvent 212 285 0 0 0 0 0 i\n"
"MouseMoveEvent 211 285 0 0 0 0 0 i\n"
"MouseMoveEvent 210 285 0 0 0 0 0 i\n"
"MouseMoveEvent 209 285 0 0 0 0 0 i\n"
"MouseMoveEvent 208 285 0 0 0 0 0 i\n"
"MouseMoveEvent 208 287 0 0 0 0 0 i\n"
"MouseMoveEvent 210 295 0 0 0 0 0 i\n"
"MouseMoveEvent 214 303 0 0 0 0 0 i\n"
"MouseMoveEvent 218 311 0 0 0 0 0 i\n"
"MouseMoveEvent 224 315 0 0 0 0 0 i\n"
"MouseMoveEvent 228 319 0 0 0 0 0 i\n"
"MouseMoveEvent 232 323 0 0 0 0 0 i\n"
"MouseMoveEvent 233 324 0 0 0 0 0 i\n"
"MouseMoveEvent 234 325 0 0 0 0 0 i\n"
"MouseMoveEvent 236 325 0 0 0 0 0 i\n"
"MouseMoveEvent 237 325 0 0 0 0 0 i\n"
"MouseMoveEvent 238 325 0 0 0 0 0 i\n"
"MouseMoveEvent 241 325 0 0 0 0 0 i\n"
"MouseMoveEvent 243 325 0 0 0 0 0 i\n"
"MouseMoveEvent 245 325 0 0 0 0 0 i\n"
"MouseMoveEvent 246 325 0 0 0 0 0 i\n"
"MouseMoveEvent 247 325 0 0 0 0 0 i\n"
"MouseMoveEvent 248 325 0 0 0 0 0 i\n"
"MouseMoveEvent 249 324 0 0 0 0 0 i\n"
"MouseMoveEvent 250 322 0 0 0 0 0 i\n"
"MouseMoveEvent 252 322 0 0 0 0 0 i\n"
"MouseMoveEvent 253 320 0 0 0 0 0 i\n"
"MouseMoveEvent 254 318 0 0 0 0 0 i\n"
"MouseMoveEvent 256 317 0 0 0 0 0 i\n"
"MouseMoveEvent 257 316 0 0 0 0 0 i\n"
"MouseMoveEvent 258 314 0 0 0 0 0 i\n"
"MouseMoveEvent 260 314 0 0 0 0 0 i\n"
"MouseMoveEvent 261 314 0 0 0 0 0 i\n"
"MouseMoveEvent 261 313 0 0 0 0 0 i\n"
"MouseMoveEvent 262 313 0 0 0 0 0 i\n"
"MouseMoveEvent 263 313 0 0 0 0 0 i\n"
"MouseMoveEvent 264 313 0 0 0 0 0 i\n"
"MouseMoveEvent 265 313 0 0 0 0 0 i\n"
"MouseMoveEvent 266 313 0 0 0 0 0 i\n"
"LeftButtonPressEvent 266 313 0 0 0 0 0 i\n"
"MouseMoveEvent 267 313 0 0 0 0 0 i\n"
"MouseMoveEvent 297 311 0 0 0 0 0 i\n"
"MouseMoveEvent 306 314 0 0 0 0 0 i\n"
"MouseMoveEvent 307 315 0 0 0 0 0 i\n"
"MouseMoveEvent 308 315 0 0 0 0 0 i\n"
"MouseMoveEvent 310 315 0 0 0 0 0 i\n"
"MouseMoveEvent 310 316 0 0 0 0 0 i\n"
"MouseMoveEvent 299 318 0 0 0 0 0 i\n"
"MouseMoveEvent 298 320 0 0 0 0 0 i\n"
"MouseMoveEvent 298 319 0 0 0 0 0 i\n"
"MouseMoveEvent 299 316 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 299 316 0 0 0 0 0 i\n"
"MouseMoveEvent 295 312 0 0 0 0 0 i\n"
"MouseMoveEvent 294 310 0 0 0 0 0 i\n"
"MouseMoveEvent 290 304 0 0 0 0 0 i\n"
"MouseMoveEvent 286 296 0 0 0 0 0 i\n"
"MouseMoveEvent 280 290 0 0 0 0 0 i\n"
"MouseMoveEvent 276 282 0 0 0 0 0 i\n"
"MouseMoveEvent 272 276 0 0 0 0 0 i\n"
"MouseMoveEvent 272 273 0 0 0 0 0 i\n"
"MouseMoveEvent 270 267 0 0 0 0 0 i\n"
"MouseMoveEvent 269 266 0 0 0 0 0 i\n"
"MouseMoveEvent 268 265 0 0 0 0 0 i\n"
"MouseMoveEvent 268 263 0 0 0 0 0 i\n"
"MouseMoveEvent 267 263 0 0 0 0 0 i\n"
"MouseMoveEvent 267 262 0 0 0 0 0 i\n"
"MouseMoveEvent 267 261 0 0 0 0 0 i\n"
"MouseMoveEvent 267 260 0 0 0 0 0 i\n"
"MouseMoveEvent 267 259 0 0 0 0 0 i\n"
"MouseMoveEvent 267 258 0 0 0 0 0 i\n"
"MouseMoveEvent 267 257 0 0 0 0 0 i\n"
"MouseMoveEvent 269 257 0 0 0 0 0 i\n"
"MouseMoveEvent 270 257 0 0 0 0 0 i\n"
"MouseMoveEvent 271 257 0 0 0 0 0 i\n"
"MouseMoveEvent 273 257 0 0 0 0 0 i\n"
"MouseMoveEvent 274 257 0 0 0 0 0 i\n"
"MouseMoveEvent 274 258 0 0 0 0 0 i\n"
"MouseMoveEvent 275 258 0 0 0 0 0 i\n"
"MouseMoveEvent 275 259 0 0 0 0 0 i\n"
"MouseMoveEvent 276 259 0 0 0 0 0 i\n"
"MouseMoveEvent 277 260 0 0 0 0 0 i\n"
"MouseMoveEvent 278 260 0 0 0 0 0 i\n"
"MouseMoveEvent 279 261 0 0 0 0 0 i\n"
"MouseMoveEvent 281 261 0 0 0 0 0 i\n"
"MouseMoveEvent 282 261 0 0 0 0 0 i\n"
"MouseMoveEvent 283 261 0 0 0 0 0 i\n"
"MouseMoveEvent 284 261 0 0 0 0 0 i\n"
"MouseMoveEvent 285 261 0 0 0 0 0 i\n"
"MouseMoveEvent 285 262 0 0 0 0 0 i\n"
"MouseMoveEvent 284 263 0 0 0 0 0 i\n"
"MouseMoveEvent 285 263 0 0 0 0 0 i\n"
"LeftButtonPressEvent 285 263 0 0 0 0 0 i\n"
"MouseMoveEvent 286 262 0 0 0 0 0 i\n"
"MouseMoveEvent 290 262 0 0 0 0 0 i\n"
"MouseMoveEvent 310 265 0 0 0 0 0 i\n"
"MouseMoveEvent 314 266 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 314 266 0 0 0 0 0 i\n"
"MouseMoveEvent 316 266 0 0 0 0 0 i\n"
"MouseMoveEvent 319 266 0 0 0 0 0 i\n"
"MouseMoveEvent 327 266 0 0 0 0 0 i\n"
"MouseMoveEvent 335 264 0 0 0 0 0 i\n"
"MouseMoveEvent 341 262 0 0 0 0 0 i\n"
"MouseMoveEvent 342 262 0 0 0 0 0 i\n"
"MouseMoveEvent 341 263 0 0 0 0 0 i\n"
"MouseMoveEvent 335 265 0 0 0 0 0 i\n"
"MouseMoveEvent 323 269 0 0 0 0 0 i\n"
"MouseMoveEvent 307 273 0 0 0 0 0 i\n"
"MouseMoveEvent 287 281 0 0 0 0 0 i\n"
"MouseMoveEvent 265 289 0 0 0 0 0 i\n"
"MouseMoveEvent 239 297 0 0 0 0 0 i\n"
"MouseMoveEvent 217 305 0 0 0 0 0 i\n"
"MouseMoveEvent 197 313 0 0 0 0 0 i\n"
"MouseMoveEvent 177 321 0 0 0 0 0 i\n"
"MouseMoveEvent 165 327 0 0 0 0 0 i\n"
"MouseMoveEvent 153 329 0 0 0 0 0 i\n"
"MouseMoveEvent 151 330 0 0 0 0 0 i\n"
"MouseMoveEvent 149 330 0 0 0 0 0 i\n"
"MouseMoveEvent 148 330 0 0 0 0 0 i\n"
"MouseMoveEvent 147 330 0 0 0 0 0 i\n"
"MouseMoveEvent 145 330 0 0 0 0 0 i\n"
"MouseMoveEvent 144 330 0 0 0 0 0 i\n"
"MouseMoveEvent 143 331 0 0 0 0 0 i\n"
"MouseMoveEvent 143 332 0 0 0 0 0 i\n"
"MouseMoveEvent 143 333 0 0 0 0 0 i\n"
"MouseMoveEvent 142 333 0 0 0 0 0 i\n"
"MouseMoveEvent 141 334 0 0 0 0 0 i\n"
"MouseMoveEvent 140 335 0 0 0 0 0 i\n"
"MouseMoveEvent 139 336 0 0 0 0 0 i\n"
"MouseMoveEvent 138 336 0 0 0 0 0 i\n"
"MouseMoveEvent 138 337 0 0 0 0 0 i\n"
"MouseMoveEvent 139 337 0 0 0 0 0 i\n"
"MouseMoveEvent 141 337 0 0 0 0 0 i\n"
"MouseMoveEvent 147 339 0 0 0 0 0 i\n"
"MouseMoveEvent 159 339 0 0 0 0 0 i\n"
"MouseMoveEvent 169 339 0 0 0 0 0 i\n"
"MouseMoveEvent 179 339 0 0 0 0 0 i\n"
"MouseMoveEvent 182 339 0 0 0 0 0 i\n"
"MouseMoveEvent 184 339 0 0 0 0 0 i\n"
"MouseMoveEvent 185 339 0 0 0 0 0 i\n"
"MouseMoveEvent 186 339 0 0 0 0 0 i\n"
"MouseMoveEvent 187 339 0 0 0 0 0 i\n"
"MouseMoveEvent 189 339 0 0 0 0 0 i\n"
"MouseMoveEvent 190 339 0 0 0 0 0 i\n"
"MouseMoveEvent 191 339 0 0 0 0 0 i\n"
"LeftButtonPressEvent 191 339 0 0 0 0 0 i\n"
"MouseMoveEvent 191 340 0 0 0 0 0 i\n"
"MouseMoveEvent 197 351 0 0 0 0 0 i\n"
"MouseMoveEvent 199 353 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 199 353 0 0 0 0 0 i\n"
"MouseMoveEvent 199 351 0 0 0 0 0 i\n"
"MouseMoveEvent 199 349 0 0 0 0 0 i\n"
"MouseMoveEvent 199 347 0 0 0 0 0 i\n"
"MouseMoveEvent 199 345 0 0 0 0 0 i\n"
"MouseMoveEvent 199 344 0 0 0 0 0 i\n"
"MouseMoveEvent 199 343 0 0 0 0 0 i\n"
"MouseMoveEvent 199 342 0 0 0 0 0 i\n"
"MouseMoveEvent 201 341 0 0 0 0 0 i\n"
"MouseMoveEvent 207 337 0 0 0 0 0 i\n"
"MouseMoveEvent 217 335 0 0 0 0 0 i\n"
"MouseMoveEvent 227 331 0 0 0 0 0 i\n"
"MouseMoveEvent 237 329 0 0 0 0 0 i\n"
"MouseMoveEvent 243 327 0 0 0 0 0 i\n"
"MouseMoveEvent 244 327 0 0 0 0 0 i\n"
"MouseMoveEvent 243 327 0 0 0 0 0 i\n"
"MouseMoveEvent 242 328 0 0 0 0 0 i\n"
"MouseMoveEvent 241 328 0 0 0 0 0 i\n"
"MouseMoveEvent 240 329 0 0 0 0 0 i\n"
"MouseMoveEvent 240 330 0 0 0 0 0 i\n"
"MouseMoveEvent 239 330 0 0 0 0 0 i\n"
"MouseMoveEvent 238 331 0 0 0 0 0 i\n"
"MouseMoveEvent 237 331 0 0 0 0 0 i\n"
"MouseMoveEvent 236 332 0 0 0 0 0 i\n"
"MouseMoveEvent 235 332 0 0 0 0 0 i\n"
"MouseMoveEvent 235 333 0 0 0 0 0 i\n"
"LeftButtonPressEvent 235 333 0 0 0 0 0 i\n"
"MouseMoveEvent 235 332 0 0 0 0 0 i\n"
"MouseMoveEvent 244 342 0 0 0 0 0 i\n"
"MouseMoveEvent 248 345 0 0 0 0 0 i\n"
"MouseMoveEvent 248 346 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 248 346 0 0 0 0 0 i\n"
"MouseMoveEvent 248 344 0 0 0 0 0 i\n"
"MouseMoveEvent 248 342 0 0 0 0 0 i\n"
"MouseMoveEvent 248 339 0 0 0 0 0 i\n"
"MouseMoveEvent 246 331 0 0 0 0 0 i\n"
"MouseMoveEvent 246 323 0 0 0 0 0 i\n"
"MouseMoveEvent 246 313 0 0 0 0 0 i\n"
"MouseMoveEvent 246 301 0 0 0 0 0 i\n"
"MouseMoveEvent 246 291 0 0 0 0 0 i\n"
"MouseMoveEvent 248 279 0 0 0 0 0 i\n"
"MouseMoveEvent 252 269 0 0 0 0 0 i\n"
"MouseMoveEvent 254 261 0 0 0 0 0 i\n"
"MouseMoveEvent 256 253 0 0 0 0 0 i\n"
"MouseMoveEvent 258 245 0 0 0 0 0 i\n"
"MouseMoveEvent 258 243 0 0 0 0 0 i\n"
"MouseMoveEvent 259 241 0 0 0 0 0 i\n"
"MouseMoveEvent 259 240 0 0 0 0 0 i\n"
"MouseMoveEvent 259 239 0 0 0 0 0 i\n"
"MouseMoveEvent 259 238 0 0 0 0 0 i\n"
"MouseMoveEvent 259 237 0 0 0 0 0 i\n"
"MouseMoveEvent 259 236 0 0 0 0 0 i\n"
"MouseMoveEvent 259 235 0 0 0 0 0 i\n"
"MouseMoveEvent 259 234 0 0 0 0 0 i\n"
"MouseMoveEvent 258 234 0 0 0 0 0 i\n"
"MouseMoveEvent 258 233 0 0 0 0 0 i\n"
"MouseMoveEvent 257 233 0 0 0 0 0 i\n"
"MouseMoveEvent 257 232 0 0 0 0 0 i\n"
"MouseMoveEvent 256 231 0 0 0 0 0 i\n"
"MouseMoveEvent 256 229 0 0 0 0 0 i\n"
"MouseMoveEvent 256 228 0 0 0 0 0 i\n"
"MouseMoveEvent 256 226 0 0 0 0 0 i\n"
"MouseMoveEvent 256 225 0 0 0 0 0 i\n"
"MouseMoveEvent 258 224 0 0 0 0 0 i\n"
"MouseMoveEvent 264 222 0 0 0 0 0 i\n"
"MouseMoveEvent 276 218 0 0 0 0 0 i\n"
"MouseMoveEvent 294 216 0 0 0 0 0 i\n"
"MouseMoveEvent 320 212 0 0 0 0 0 i\n"
"MouseMoveEvent 348 210 0 0 0 0 0 i\n"
"MouseMoveEvent 380 208 0 0 0 0 0 i\n"
"MouseMoveEvent 410 206 0 0 0 0 0 i\n"
"MouseMoveEvent 438 206 0 0 0 0 0 i\n"
"MouseMoveEvent 462 206 0 0 0 0 0 i\n"
"MouseMoveEvent 478 206 0 0 0 0 0 i\n"
"MouseMoveEvent 490 206 0 0 0 0 0 i\n"
"MouseMoveEvent 493 206 0 0 0 0 0 i\n"
"MouseMoveEvent 499 208 0 0 0 0 0 i\n"
"MouseMoveEvent 502 208 0 0 0 0 0 i\n"
"MouseMoveEvent 503 209 0 0 0 0 0 i\n"
"MouseMoveEvent 504 209 0 0 0 0 0 i\n"
"MouseMoveEvent 505 210 0 0 0 0 0 i\n"
"MouseMoveEvent 506 211 0 0 0 0 0 i\n"
"MouseMoveEvent 508 212 0 0 0 0 0 i\n"
"MouseMoveEvent 510 212 0 0 0 0 0 i\n"
"MouseMoveEvent 512 212 0 0 0 0 0 i\n"
"MouseMoveEvent 515 212 0 0 0 0 0 i\n"
"MouseMoveEvent 516 212 0 0 0 0 0 i\n"
"MouseMoveEvent 517 212 0 0 0 0 0 i\n"
"MouseMoveEvent 518 212 0 0 0 0 0 i\n"
"MouseMoveEvent 519 212 0 0 0 0 0 i\n"
"MouseMoveEvent 522 212 0 0 0 0 0 i\n"
"MouseMoveEvent 524 212 0 0 0 0 0 i\n"
"MouseMoveEvent 530 210 0 0 0 0 0 i\n"
"MouseMoveEvent 538 208 0 0 0 0 0 i\n"
"MouseMoveEvent 541 208 0 0 0 0 0 i\n"
"MouseMoveEvent 547 206 0 0 0 0 0 i\n"
"MouseMoveEvent 549 206 0 0 0 0 0 i\n"
"MouseMoveEvent 550 206 0 0 0 0 0 i\n"
"MouseMoveEvent 551 206 0 0 0 0 0 i\n"
"MouseMoveEvent 552 206 0 0 0 0 0 i\n"
"MouseMoveEvent 553 206 0 0 0 0 0 i\n"
"MouseMoveEvent 554 206 0 0 0 0 0 i\n"
"MouseMoveEvent 554 207 0 0 0 0 0 i\n"
"MouseMoveEvent 554 208 0 0 0 0 0 i\n"
"MouseMoveEvent 554 209 0 0 0 0 0 i\n"
"MouseMoveEvent 555 210 0 0 0 0 0 i\n"
"LeftButtonPressEvent 555 210 0 0 0 0 0 i\n"
"MouseMoveEvent 555 209 0 0 0 0 0 i\n"
"MouseMoveEvent 550 173 0 0 0 0 0 i\n"
"MouseMoveEvent 544 155 0 0 0 0 0 i\n"
"MouseMoveEvent 543 154 0 0 0 0 0 i\n"
"MouseMoveEvent 542 152 0 0 0 0 0 i\n"
"MouseMoveEvent 533 135 0 0 0 0 0 i\n"
"MouseMoveEvent 519 122 0 0 0 0 0 i\n"
"MouseMoveEvent 511 115 0 0 0 0 0 i\n"
"MouseMoveEvent 510 114 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 510 114 0 0 0 0 0 i\n"
"MouseMoveEvent 512 115 0 0 0 0 0 i\n"
"MouseMoveEvent 513 117 0 0 0 0 0 i\n"
"MouseMoveEvent 515 118 0 0 0 0 0 i\n"
"MouseMoveEvent 521 124 0 0 0 0 0 i\n"
"MouseMoveEvent 525 132 0 0 0 0 0 i\n"
"MouseMoveEvent 529 138 0 0 0 0 0 i\n"
"MouseMoveEvent 530 140 0 0 0 0 0 i\n"
"MouseMoveEvent 532 146 0 0 0 0 0 i\n"
"MouseMoveEvent 533 148 0 0 0 0 0 i\n"
"MouseMoveEvent 534 150 0 0 0 0 0 i\n"
"MouseMoveEvent 534 153 0 0 0 0 0 i\n"
"MouseMoveEvent 534 154 0 0 0 0 0 i\n"
"MouseMoveEvent 534 157 0 0 0 0 0 i\n"
"MouseMoveEvent 534 158 0 0 0 0 0 i\n"
"MouseMoveEvent 534 159 0 0 0 0 0 i\n"
"MouseMoveEvent 534 161 0 0 0 0 0 i\n"
"MouseMoveEvent 534 162 0 0 0 0 0 i\n"
"MouseMoveEvent 534 163 0 0 0 0 0 i\n"
"MouseMoveEvent 534 164 0 0 0 0 0 i\n"
"MouseMoveEvent 534 165 0 0 0 0 0 i\n"
"MouseMoveEvent 533 165 0 0 0 0 0 i\n"
"MouseMoveEvent 532 165 0 0 0 0 0 i\n"
"MouseMoveEvent 531 165 0 0 0 0 0 i\n"
"MouseMoveEvent 530 165 0 0 0 0 0 i\n"
"MouseMoveEvent 529 165 0 0 0 0 0 i\n"
"MouseMoveEvent 529 166 0 0 0 0 0 i\n"
"MouseMoveEvent 528 167 0 0 0 0 0 i\n"
"MouseMoveEvent 527 167 0 0 0 0 0 i\n"
"MouseMoveEvent 527 168 0 0 0 0 0 i\n"
"MouseMoveEvent 526 168 0 0 0 0 0 i\n"
"MouseMoveEvent 526 169 0 0 0 0 0 i\n"
"LeftButtonPressEvent 526 169 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 526 169 0 0 0 0 0 i\n"
"MouseMoveEvent 525 166 0 0 0 0 0 i\n"
"MouseMoveEvent 525 165 0 0 0 0 0 i\n"
"MouseMoveEvent 525 164 0 0 0 0 0 i\n"
"MouseMoveEvent 525 163 0 0 0 0 0 i\n"
"MouseMoveEvent 525 162 0 0 0 0 0 i\n"
"MouseMoveEvent 525 161 0 0 0 0 0 i\n"
"MouseMoveEvent 525 162 0 0 0 0 0 i\n"
"MouseMoveEvent 526 163 0 0 0 0 0 i\n"
"MouseMoveEvent 526 164 0 0 0 0 0 i\n"
"MouseMoveEvent 526 165 0 0 0 0 0 i\n"
"MouseMoveEvent 526 166 0 0 0 0 0 i\n"
"MouseMoveEvent 526 167 0 0 0 0 0 i\n"
"MouseMoveEvent 526 169 0 0 0 0 0 i\n"
"MouseMoveEvent 526 170 0 0 0 0 0 i\n"
"MouseMoveEvent 526 171 0 0 0 0 0 i\n"
"MouseMoveEvent 526 172 0 0 0 0 0 i\n"
"MouseMoveEvent 526 173 0 0 0 0 0 i\n"
"LeftButtonPressEvent 526 173 0 0 0 0 0 i\n"
"MouseMoveEvent 526 174 0 0 0 0 0 i\n"
"MouseMoveEvent 510 179 0 0 0 0 0 i\n"
"LeftButtonReleaseEvent 510 179 0 0 0 0 0 i\n"
"MouseMoveEvent 510 177 0 0 0 0 0 i\n"
"MouseMoveEvent 510 175 0 0 0 0 0 i\n"
"MouseMoveEvent 510 172 0 0 0 0 0 i\n"
"MouseMoveEvent 512 164 0 0 0 0 0 i\n"
"MouseMoveEvent 512 154 0 0 0 0 0 i\n"
"MouseMoveEvent 514 142 0 0 0 0 0 i\n"
"MouseMoveEvent 514 134 0 0 0 0 0 i\n"
"MouseMoveEvent 514 131 0 0 0 0 0 i\n"
"MouseMoveEvent 513 129 0 0 0 0 0 i\n"
"MouseMoveEvent 513 127 0 0 0 0 0 i\n"
"MouseMoveEvent 512 126 0 0 0 0 0 i\n"
"MouseMoveEvent 510 125 0 0 0 0 0 i\n"
"MouseMoveEvent 509 124 0 0 0 0 0 i\n"
"MouseMoveEvent 507 118 0 0 0 0 0 i\n"
"MouseMoveEvent 503 114 0 0 0 0 0 i\n"
"MouseMoveEvent 502 112 0 0 0 0 0 i\n"
"MouseMoveEvent 501 112 0 0 0 0 0 i\n"
"MouseMoveEvent 500 111 0 0 0 0 0 i\n"
"MouseMoveEvent 499 111 0 0 0 0 0 i\n"
"MouseMoveEvent 499 110 0 0 0 0 0 i\n"
"MouseMoveEvent 499 112 0 0 0 0 0 i\n"
"MouseMoveEvent 501 118 0 0 0 0 0 i\n"
"MouseMoveEvent 501 120 0 0 0 0 0 i\n"
"MouseMoveEvent 501 121 0 0 0 0 0 i\n"
"MouseMoveEvent 500 121 0 0 0 0 0 i\n"
"MouseMoveEvent 500 120 0 0 0 0 0 i\n"
"MouseMoveEvent 499 119 0 0 0 0 0 i\n"
"MouseMoveEvent 497 119 0 0 0 0 0 i\n"
"MouseMoveEvent 495 118 0 0 0 0 0 i\n"
"MouseMoveEvent 487 114 0 0 0 0 0 i\n"
"MouseMoveEvent 481 112 0 0 0 0 0 i\n"
"MouseMoveEvent 471 110 0 0 0 0 0 i\n"
"MouseMoveEvent 461 108 0 0 0 0 0 i\n"
"MouseMoveEvent 453 106 0 0 0 0 0 i\n"
"MouseMoveEvent 445 104 0 0 0 0 0 i\n"
"MouseMoveEvent 435 104 0 0 0 0 0 i\n"
"MouseMoveEvent 432 104 0 0 0 0 0 i\n"
"MouseMoveEvent 430 104 0 0 0 0 0 i\n"
"MouseMoveEvent 429 104 0 0 0 0 0 i\n"
"MouseMoveEvent 425 108 0 0 0 0 0 i\n"
"MouseMoveEvent 424 109 0 0 0 0 0 i\n"
"MouseMoveEvent 422 109 0 0 0 0 0 i\n"
"MouseMoveEvent 421 111 0 0 0 0 0 i\n"
"MouseMoveEvent 417 115 0 0 0 0 0 i\n"
"MouseMoveEvent 413 119 0 0 0 0 0 i\n"
"MouseMoveEvent 411 120 0 0 0 0 0 i\n"
"MouseMoveEvent 410 121 0 0 0 0 0 i\n"
"MouseMoveEvent 409 121 0 0 0 0 0 i\n"
"MouseMoveEvent 409 122 0 0 0 0 0 i\n"
"MouseMoveEvent 409 123 0 0 0 0 0 i\n"
"MouseMoveEvent 409 124 0 0 0 0 0 i\n"
"MouseMoveEvent 408 124 0 0 0 0 0 i\n"
"MouseMoveEvent 408 125 0 0 0 0 0 i\n"
"MouseMoveEvent 407 125 0 0 0 0 0 i\n"
"MouseMoveEvent 403 129 0 0 0 0 0 i\n"
"MouseMoveEvent 403 130 0 0 0 0 0 i\n"
"MouseMoveEvent 402 130 0 0 0 0 0 i\n"
"KeyPressEvent 402 130 0 0 113 1 q i\n"
"CharEvent 402 130 0 0 113 1 q i\n"
"ExitEvent 402 130 0 0 113 1 q i\n";

int TerrainPolylineEditor(int argc, char * argv[])
{
  if (argc < 2)
    {
    std::cerr
    << "Demonstrates editing capabilities of a contour widget on terrain \n"
    << "data. Additional arguments : \n"
    << "\tThe projection mode may optionally be specified. [0-Simple,1-NonOccluded\n"
    << ",2-Hug]. (defaults to Hug)\n"
    << "\tA height offset may be specified. Defaults to 0.0\n"
    << "\tIf a polydata is specified, an initial contour is constucted from\n"
    << "the points in the polydata. The polydata is expected to be a polyline\n"
    << "(one cell and two or more points on that cell)."
    << std::endl;
    std::cerr << "\n\nUsage: " << argv[0] << "\n"
              << "  [-ProjectionMode (0,1 or 2)]\n"
              << "  [-HeightOffset heightOffset]\n"
              << "  [-InitialPath SomeVTKXmlfileContainingPath.vtk]"
              << std::endl;
    return EXIT_FAILURE;
    }

  // Read height field.
  char* fname =
    vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/SainteHelens.dem");
  vtkSmartPointer<vtkDEMReader> demReader =
    vtkSmartPointer<vtkDEMReader>::New();
  demReader->SetFileName(fname);
  delete [] fname;

  // Extract geometry

  vtkSmartPointer<vtkImageDataGeometryFilter> surface =
    vtkSmartPointer<vtkImageDataGeometryFilter>::New();
  surface->SetInput(demReader->GetOutput());

  vtkSmartPointer<vtkWarpScalar> warp =
    vtkSmartPointer<vtkWarpScalar>::New();
  warp->SetInput(surface->GetOutput());
  warp->SetScaleFactor(1);
  warp->UseNormalOn();
  warp->SetNormal(0, 0, 1);
  warp->Update();

  // Define a LUT mapping for the height field

  double lo = demReader->GetOutput()->GetScalarRange()[0];
  double hi = demReader->GetOutput()->GetScalarRange()[1];

  vtkSmartPointer<vtkLookupTable> lut =
    vtkSmartPointer<vtkLookupTable>::New();
  lut->SetHueRange(0.6, 0);
  lut->SetSaturationRange(1.0, 0);
  lut->SetValueRange(0.5, 1.0);

  vtkSmartPointer<vtkPolyDataNormals> normals =
    vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInput(warp->GetPolyDataOutput());
  normals->SetFeatureAngle(60);
  normals->SplittingOff();

  vtkSmartPointer<vtkPolyDataMapper> demMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  demMapper->SetInput(normals->GetOutput());
  normals->Update();
  demMapper->SetScalarRange(lo, hi);
  demMapper->SetLookupTable(lut);

  vtkSmartPointer<vtkActor> demActor =
    vtkSmartPointer<vtkActor>::New();
  demActor->SetMapper(demMapper);

  // Create the RenderWindow, Renderer and the DEM + path actors.

  vtkSmartPointer<vtkRenderer> ren1 =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren1);
  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  // Add the actors to the renderer, set the background and size

  renWin->SetSize(600,600);
  ren1->AddActor(demActor);
  ren1->GetActiveCamera()->SetViewUp(0, 0, 1);
  ren1->GetActiveCamera()->SetPosition(-99900, -21354, 131801);
  ren1->GetActiveCamera()->SetFocalPoint(41461, 41461, 2815);
  ren1->ResetCamera();
  ren1->GetActiveCamera()->Dolly(1.2);
  ren1->ResetCameraClippingRange();

  // Here comes the contour widget stuff.....

  vtkSmartPointer<vtkContourWidget> contourWidget =
    vtkSmartPointer<vtkContourWidget>::New();
  vtkOrientedGlyphContourRepresentation *rep =
      vtkOrientedGlyphContourRepresentation::SafeDownCast(
                        contourWidget->GetRepresentation());
  rep->GetLinesProperty()->SetColor(1.0, 0.0, 0.0);
  contourWidget->SetInteractor(iren);

  // Set the point placer to the one used for terrains...

  vtkSmartPointer<vtkTerrainDataPointPlacer>  pointPlacer =
    vtkSmartPointer<vtkTerrainDataPointPlacer>::New();
  pointPlacer->AddProp(demActor);    // the actor(s) containing the terrain.
  rep->SetPointPlacer(pointPlacer);

  // Set a terrain interpolator. Interpolates points as they are placed,
  // so that they lie on the terrain.

  vtkSmartPointer<vtkTerrainContourLineInterpolator> interpolator
          = vtkSmartPointer<vtkTerrainContourLineInterpolator>::New();
  rep->SetLineInterpolator(interpolator);
  interpolator->SetImageData(demReader->GetOutput());

  // Set the default projection mode to hug the terrain, unless user
  // overrides it.
  //
  interpolator->GetProjector()->SetProjectionModeToHug();
  for (int i = 0; i < argc-1; i++)
    {
    if (strcmp("-ProjectionMode", argv[i]) == 0)
      {
      interpolator->GetProjector()->SetProjectionMode(atoi(argv[i+1]));
      }
    if (strcmp("-HeightOffset", argv[i]) == 0)
      {
      interpolator->GetProjector()->SetHeightOffset(atoi(argv[i+1]));
      pointPlacer->SetHeightOffset(atof(argv[i+1]));
      }
    if (strcmp("-InitialPath", argv[i]) == 0)
      {
      // If we had an input poly as an initial path, build a contour
      // widget from that path.
      //
      vtkSmartPointer<vtkPolyDataReader> terrainPathReader =
        vtkSmartPointer<vtkPolyDataReader>::New();
      terrainPathReader->SetFileName(argv[i+1]);
      terrainPathReader->Update();
      contourWidget->Initialize( terrainPathReader->GetOutput(), 0 );
      }
    }

  contourWidget->EnabledOn();

  vtkSmartPointer<vtkInteractorEventRecorder> recorder =
    vtkSmartPointer<vtkInteractorEventRecorder>::New();
  recorder->SetInteractor(iren);
  recorder->ReadFromInputStringOn();
  recorder->SetInputString(TerrainPolylineEditorLog);
  recorder->EnabledOn();

  renWin->Render();
  iren->Initialize();

  recorder->Play();

  recorder->Off();

  iren->Start();

  return EXIT_SUCCESS;
}
