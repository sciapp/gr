#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gr3.h"
#include "gr3_internals.h"
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'};
static unsigned int mod_table[] = {0, 2, 1};


static char *base64_encode(const unsigned char *data,
                    size_t input_length) {
  unsigned int i, j;
  size_t output_length = 4 * ((input_length + 2) / 3) + 1;
  char *encoded_data = malloc(output_length);
  if (encoded_data == NULL) {
    return NULL;
  }
  
  for (i = 0, j = 0; i < input_length;) {
    unsigned int octet_a = i < input_length ? data[i++] : 0;
    unsigned int octet_b = i < input_length ? data[i++] : 0;
    unsigned int octet_c = i < input_length ? data[i++] : 0;
    
    unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
    
    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }
  
  for (i = 0; i < mod_table[input_length % 3]; i++) {
    encoded_data[output_length - 2 - i] = '=';
  }
  encoded_data[output_length-1] = 0;
  return encoded_data;
}

int gr3_export_html_(const char *filename, int width, int height) {
  FILE *htmlfp;
  const char *title = "GR3";
  int i, j;
  char *b64vertices = NULL, *b64normals = NULL;
  
  htmlfp = fopen(filename, "w");
  if (!htmlfp) {
    RETURN_ERROR(GR3_ERROR_CANNOT_OPEN_FILE);
  }
  fprintf(htmlfp, "<!DOCTYPE html>\n");
  fprintf(htmlfp, "<html>\n");
  fprintf(htmlfp, "  <head>\n");
  fprintf(htmlfp, "    <meta charset=\"utf-8\" />\n");
  fprintf(htmlfp, "    <title>%s</title>\n", title);
  fprintf(htmlfp, "    <script type=\"text/javascript\">\n");
  fprintf(htmlfp, "      function b64ToUint6 (nChr) {\n");
  fprintf(htmlfp, "        return nChr > 64 && nChr < 91 ? nChr - 65 : nChr > 96 && nChr < 123 ? nChr - 71 : nChr > 47 && nChr < 58 ? nChr + 4 : nChr === 43 ? 62 : nChr === 47 ? 63 : 0;\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "      function base64DecToArr (sBase64, nBlocksSize) {\n");
  fprintf(htmlfp, "    var sB64Enc = sBase64.replace(/[^A-Za-z0-9\\+\\/]/g, \"\"), nInLen = sB64Enc.length, nOutLen = nBlocksSize ? Math.ceil((nInLen * 3 + 1 >> 2) / nBlocksSize) * nBlocksSize : nInLen * 3 + 1 >> 2, taBytes = new Uint8Array(nOutLen);\n");
  fprintf(htmlfp, "    for (var nMod3, nMod4, nUint24 = 0, nOutIdx = 0, nInIdx = 0; nInIdx < nInLen; nInIdx++) {\n");
  fprintf(htmlfp, "      nMod4 = nInIdx & 3;\n");
  fprintf(htmlfp, "      nUint24 |= b64ToUint6(sB64Enc.charCodeAt(nInIdx)) << 18 - 6 * nMod4;\n");
  fprintf(htmlfp, "      if (nMod4 === 3 || nInLen - nInIdx === 1) {\n");
  fprintf(htmlfp, "        for (nMod3 = 0; nMod3 < 3 && nOutIdx < nOutLen; nMod3++, nOutIdx++) {\n");
  fprintf(htmlfp, "          taBytes[nOutIdx] = nUint24 >>> (16 >>> nMod3 & 24) & 255;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        nUint24 = 0;\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "    }\n");
  fprintf(htmlfp, "    return taBytes;\n");
  fprintf(htmlfp, "  }\n");
  
  fprintf(htmlfp, "      function startWebGLCanvas() {\n");
  fprintf(htmlfp, "        var canvas = document.getElementById(\"webgl-canvas\");\n");
  fprintf(htmlfp, "        initWebGL(canvas);\n");
  fprintf(htmlfp, "        initShaderProgram();\n");
  fprintf(htmlfp, "        initMeshes();\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        gl.enable(gl.DEPTH_TEST);\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        drawScene();\n");
  fprintf(htmlfp, "        canvas.onmousemove = canvasMouseMove;\n");
  fprintf(htmlfp, "        canvas.onmouseup = canvasMouseUp;\n");
  fprintf(htmlfp, "        canvas.onmousedown = canvasMouseDown;\n");
  fprintf(htmlfp, "        canvas.onmouseout = canvasMouseOut;\n");
  fprintf(htmlfp, "        canvas.onkeypress = canvasKeyPress;\n");
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "      function transposeMatrix4(matrix) {\n");
  fprintf(htmlfp, "        var transposedMatrix = [");
  fprintf(htmlfp, "          matrix[0],  matrix[4],  matrix[8],  matrix[12],\n");
  fprintf(htmlfp, "          matrix[1],  matrix[5],  matrix[9],  matrix[13],\n");
  fprintf(htmlfp, "          matrix[2],  matrix[6],  matrix[10], matrix[14],\n");
  fprintf(htmlfp, "          matrix[3],  matrix[7],  matrix[11], matrix[15]\n");
  fprintf(htmlfp, "        ];\n");
  fprintf(htmlfp, "        return transposedMatrix;\n");
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "      var gl;\n");
  fprintf(htmlfp, "      function initWebGL(canvas) {\n");
  fprintf(htmlfp, "        try {\n");
  fprintf(htmlfp, "          gl = canvas.getContext(\"experimental-webgl\", {antialias: true, stencil: false});\n");
  fprintf(htmlfp, "          gl.viewportWidth = canvas.width;\n");
  fprintf(htmlfp, "          gl.viewportHeight = canvas.height;\n");
  fprintf(htmlfp, "        } catch(e) {\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        if (!gl) {\n");
  fprintf(htmlfp, "          alert(\"Unable to initialize WebGL.\");\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "      }\n");
  
  
  fprintf(htmlfp, "      var meshes;\n");
  fprintf(htmlfp, "      function initMeshes() {\n");
  fprintf(htmlfp, "        function Mesh(id, vertices, normals, colors) {\n");
  fprintf(htmlfp, "          this.id = id;\n");
  fprintf(htmlfp, "          this.vertices = vertices;\n");
  fprintf(htmlfp, "          this.normals = normals;\n");
  fprintf(htmlfp, "          this.colors = colors;\n");
  fprintf(htmlfp, "          \n");
  fprintf(htmlfp, "          this.init = function () {\n");
  fprintf(htmlfp, "            this.vertex_buffer = gl.createBuffer()\n");
  fprintf(htmlfp, "            gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);\n");
  fprintf(htmlfp, "            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.vertices), gl.STATIC_DRAW);\n");
  fprintf(htmlfp, "            this.normal_buffer = gl.createBuffer()\n");
  fprintf(htmlfp, "            gl.bindBuffer(gl.ARRAY_BUFFER, this.normal_buffer);\n");
  fprintf(htmlfp, "            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.normals), gl.STATIC_DRAW);\n");
  fprintf(htmlfp, "            this.color_buffer = gl.createBuffer()\n");
  fprintf(htmlfp, "            gl.bindBuffer(gl.ARRAY_BUFFER, this.color_buffer);\n");
  fprintf(htmlfp, "            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.colors), gl.STATIC_DRAW);\n");
  fprintf(htmlfp, "            this.number_of_vertices = vertices.length/3;\n");
  fprintf(htmlfp, "            \n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          this.bind = function () {\n");
  fprintf(htmlfp, "            gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);\n");
  fprintf(htmlfp, "            gl.vertexAttribPointer(shaderProgram.vertexLocation, 3, gl.FLOAT, false, 0, 0);\n");
  fprintf(htmlfp, "            gl.enableVertexAttribArray(shaderProgram.vertexLocation);\n");
  fprintf(htmlfp, "            gl.bindBuffer(gl.ARRAY_BUFFER, this.normal_buffer);\n");
  fprintf(htmlfp, "            gl.vertexAttribPointer(shaderProgram.normalLocation, 3, gl.FLOAT, false, 0, 0);\n");
  fprintf(htmlfp, "            gl.enableVertexAttribArray(shaderProgram.normalLocation);\n");
  fprintf(htmlfp, "            gl.bindBuffer(gl.ARRAY_BUFFER, this.color_buffer);\n");
  fprintf(htmlfp, "            gl.vertexAttribPointer(shaderProgram.colorLocation, 3, gl.FLOAT, false, 0, 0);\n");
  fprintf(htmlfp, "            gl.enableVertexAttribArray(shaderProgram.colorLocation);\n");
  fprintf(htmlfp, "            \n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          this.draw = function (projectionMatrix, viewMatrix, modelMatrix, scales, lightDirection) {\n");
  fprintf(htmlfp, "            gl.uniformMatrix4fv(shaderProgram.projectionMatrixLocation, false, new Float32Array(projectionMatrix));\n");
  fprintf(htmlfp, "            gl.uniformMatrix4fv(shaderProgram.viewMatrixLocation, false, new Float32Array(viewMatrix));\n");
  fprintf(htmlfp, "            gl.uniformMatrix4fv(shaderProgram.modelMatrixLocation, false, new Float32Array(modelMatrix));\n");
  fprintf(htmlfp, "            gl.uniform3fv(shaderProgram.scalesLocation, new Float32Array(scales));\n");
  fprintf(htmlfp, "            gl.uniform3fv(shaderProgram.lightDirectionLocation, new Float32Array(lightDirection));\n");
  fprintf(htmlfp, "            \n");
  fprintf(htmlfp, "            gl.drawArrays(gl.TRIANGLES, 0, this.number_of_vertices);\n");
  fprintf(htmlfp, "            \n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        meshes = new Array();\n");
  for (i = 0; i < context_struct_.mesh_list_capacity_; i++) {
    if (context_struct_.mesh_list_[i].refcount > 0) {
      gr3_sortindexedmeshdata(i);
      b64vertices = base64_encode((unsigned char *)context_struct_.mesh_list_[i].data.vertices, context_struct_.mesh_list_[i].data.number_of_vertices*3*sizeof(float));
      fprintf(htmlfp, "        var vertices = new Float32Array(base64DecToArr('%s').buffer, 0, %d);", b64vertices, context_struct_.mesh_list_[i].data.number_of_vertices*3);
      
      b64normals = base64_encode((unsigned char *)context_struct_.mesh_list_[i].data.normals, context_struct_.mesh_list_[i].data.number_of_vertices*3*sizeof(float));
      fprintf(htmlfp, "        var normals = new Float32Array(base64DecToArr('%s').buffer, 0, %d);", b64normals, context_struct_.mesh_list_[i].data.number_of_vertices*3);
      {
        int all_ones = 1;
        for (j = 0; j < context_struct_.mesh_list_[i].data.number_of_vertices*3; j++) {
          if (context_struct_.mesh_list_[i].data.colors[j] != 1) {
            all_ones = 0;
            break;
          }
        }
        if (!all_ones) {
          char *b64colors = base64_encode((unsigned char *)context_struct_.mesh_list_[i].data.colors, context_struct_.mesh_list_[i].data.number_of_vertices*3*sizeof(float));
          fprintf(htmlfp, "        var colors = new Float32Array(base64DecToArr('%s').buffer, 0, %d);", b64colors, context_struct_.mesh_list_[i].data.number_of_vertices*3);
          
        } else {
          fprintf(htmlfp, "        var colors = Array();");
          fprintf(htmlfp, "        for (var i = 0; i < %d; i++) {", context_struct_.mesh_list_[i].data.number_of_vertices*3);
          fprintf(htmlfp, "          colors[i] = 1.0;");
          fprintf(htmlfp, "        }");
        }
        fprintf(htmlfp, "        \n");
      }
      fprintf(htmlfp, "        var mesh = new Mesh(%u, vertices, normals, colors);\n", i);
      fprintf(htmlfp, "        mesh.init();\n");
      fprintf(htmlfp, "        meshes.push(mesh);\n");
    }
  }
  fprintf(htmlfp, "      }\n");
  
  
  
  fprintf(htmlfp, "      function getRotationMatrix(angle, x, y, z) {\n");
  fprintf(htmlfp, "        var f = Math.PI/180;\n");
  fprintf(htmlfp, "        var s = Math.sin(angle);\n");
  fprintf(htmlfp, "        var c = Math.cos(angle);\n");
  fprintf(htmlfp, "        var matrix = [x*x*(1-c)+c,     x*y*(1-c)-z*s,   x*z*(1-c)+y*s, 0,\n");
  fprintf(htmlfp, "                      y*x*(1-c)+z*s,   y*y*(1-c)+c,     y*z*(1-c)-x*s, 0,\n");
  fprintf(htmlfp, "                      x*z*(1-c)-y*s,   y*z*(1-c)+x*s,   z*z*(1-c)+c,   0,\n");
  fprintf(htmlfp, "                                  0,               0,             0,   1];\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        return matrix;\n");
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "      function multMatrix4(matrix1, matrix2) {\n");
  fprintf(htmlfp, "        var matrix = [0,0,0,0,\n");
  fprintf(htmlfp, "                      0,0,0,0,\n");
  fprintf(htmlfp, "                      0,0,0,0,\n");
  fprintf(htmlfp, "                      0,0,0,0];\n");
  fprintf(htmlfp, "        var i, j, k;\n");
  fprintf(htmlfp, "        for (i = 0; i < 4; i++) {\n");
  fprintf(htmlfp, "          for (j = 0; j < 4; j++) {\n");
  fprintf(htmlfp, "            matrix[i+4*j] = 0;\n");
  fprintf(htmlfp, "            for (k = 0; k < 4; k++) {\n");
  fprintf(htmlfp, "              matrix[i+4*j] = matrix[i+4*j] + matrix1[j*4+k]*matrix2[k*4+i];\n");
  fprintf(htmlfp, "            }\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        return matrix;\n");
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "      var isDragging = false;\n");
  fprintf(htmlfp, "      var xOffset = 0;\n");
  fprintf(htmlfp, "      var yOffset = 0;\n");
  fprintf(htmlfp, "      function canvasMouseUp(event) {\n");
  fprintf(htmlfp, "        isDragging = false;\n");
  fprintf(htmlfp, "        xOffset = event.clientX;\n");
  fprintf(htmlfp, "        yOffset = event.clientY;\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "      function canvasMouseDown(event) {\n");
  fprintf(htmlfp, "        isDragging = true;\n");
  fprintf(htmlfp, "        xOffset = event.clientX;\n");
  fprintf(htmlfp, "        yOffset = event.clientY;\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "      function canvasMouseOut(event) {\n");
  fprintf(htmlfp, "        isDragging = false;\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "      function canvasKeyPress(event) {\n");
  fprintf(htmlfp, "        var unicode=event.keyCode? event.keyCode : event.charCode;\n");
  fprintf(htmlfp, "        var character = String.fromCharCode(unicode);\n");
  fprintf(htmlfp, "        if (character == \"r\") {\n");
  fprintf(htmlfp, "          camera_pos = original_camera_pos.slice(0);\n");
  fprintf(htmlfp, "          center_pos = original_center_pos.slice(0);\n");
  fprintf(htmlfp, "          up_dir = original_up_dir.slice(0);\n");
  fprintf(htmlfp, "          calculateViewMatrix();\n");
  fprintf(htmlfp, "          drawScene();\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "      function canvasMouseMove(event) {\n");
  fprintf(htmlfp, "        if (isDragging) {\n");
  fprintf(htmlfp, "          dx = event.clientX-xOffset;\n");
  fprintf(htmlfp, "          dy = event.clientY-yOffset;\n");
  fprintf(htmlfp, "          if (dx == 0 && dy == 0) return;\n");
  fprintf(htmlfp, "          xOffset = event.clientX;\n");
  fprintf(htmlfp, "          yOffset = event.clientY;\n");
  fprintf(htmlfp, "          var forward = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            forward[i] = center_pos[i] - camera_pos[i];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          var up = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "          var tmp = 0;\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            tmp = tmp + forward[i]*forward[i];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          var len_forward = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "          var tmp = 0;\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            tmp = tmp + up_dir[i]*up_dir[i];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            up[i] = up_dir[i]/tmp;\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          var right = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            right[i] = forward[(i+1)%%3]*up[(i+2)%%3] - up[(i+1)%%3]*forward[(i+2)%%3];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          var tmp = 0;\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            tmp = tmp + right[i]*right[i];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            right[i] = right[i]/tmp;\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          \n");
  fprintf(htmlfp, "          var rotation = [0.0, 0.0, 0.0];\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            rotation[i] = dx*up[i]+dy*right[i];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          var tmp = 0;\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            tmp = tmp + rotation[i]*rotation[i];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            rotation[i] = rotation[i]/tmp;\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          rotationsMatrix = getRotationMatrix(-Math.sqrt(dx*dx+dy*dy)*0.003, rotation[0], rotation[1], rotation[2])\n");
  fprintf(htmlfp, "          viewMatrix = multMatrix4(rotationsMatrix, viewMatrix);\n");
  fprintf(htmlfp, "          up_dir = [viewMatrix[1], viewMatrix[5], viewMatrix[9]];\n");
  fprintf(htmlfp, "          forward = [viewMatrix[2], viewMatrix[6], viewMatrix[10]];\n");
  fprintf(htmlfp, "          for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "            camera_pos[i] = center_pos[i]+len_forward*forward[i]\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          drawScene();\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "      }\n");
  
  
  fprintf(htmlfp, "      viewMatrix = null;\n");
  
  fprintf(htmlfp, "      function calculateViewMatrix() {\n");
  fprintf(htmlfp, "        viewMatrix = [\n");
  fprintf(htmlfp, "          0.0, 0.0, 0.0, 0.0,\n");
  fprintf(htmlfp, "          0.0, 0.0, 0.0, 0.0,\n");
  fprintf(htmlfp, "          0.0, 0.0, 0.0, 0.0,\n");
  fprintf(htmlfp, "          0.0, 0.0, 0.0, 0.0\n");
  fprintf(htmlfp, "        ];\n");
  fprintf(htmlfp, "        var i = 0; var j = 0;\n");
  fprintf(htmlfp, "        var F = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "        var f = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          F[i] = center_pos[i] - camera_pos[i];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var tmp = 0;\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          tmp = tmp + F[i]*F[i];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          f[i] = F[i]/tmp;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var up = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "        var tmp = 0;\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          tmp = tmp + up_dir[i]*up_dir[i];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          up[i] = up_dir[i]/tmp;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var s = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          s[i] = f[(i+1)%%3]*up[(i+2)%%3] - up[(i+1)%%3]*f[(i+2)%%3];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var tmp = 0;\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          tmp = tmp + s[i]*s[i];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          s[i] = s[i]/tmp;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var u = [0.0 ,0.0, 0.0];\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          u[i] = s[(i+1)%%3]*f[(i+2)%%3] - f[(i+1)%%3]*s[(i+2)%%3];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var tmp = 0;\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          tmp = tmp + u[i]*u[i];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        tmp = Math.sqrt(tmp);\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          u[i] = u[i]/tmp;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          viewMatrix[i+0] = s[i];\n");
  fprintf(htmlfp, "          viewMatrix[i+4] = u[i];\n");
  fprintf(htmlfp, "          viewMatrix[i+8] = -f[i];\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        viewMatrix[15] = 1;\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        for (i = 0; i < 3; i++) {\n");
  fprintf(htmlfp, "          viewMatrix[3+4*i] = 0;\n");
  fprintf(htmlfp, "          for (j = 0; j < 3; j++) {\n");
  fprintf(htmlfp, "            viewMatrix[3+4*i] = viewMatrix[3+4*i] - viewMatrix[j+4*i]*camera_pos[j];\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        viewMatrix = transposeMatrix4(viewMatrix);\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "      var camera_pos = [%g, %g, %g];\n", context_struct_.camera_x, context_struct_.camera_y, context_struct_.camera_z);
  fprintf(htmlfp, "      var center_pos = [%g, %g, %g];\n", context_struct_.center_x, context_struct_.center_y, context_struct_.center_z);
  fprintf(htmlfp, "      var up_dir = [%g, %g, %g];\n", context_struct_.up_x, context_struct_.up_y, context_struct_.up_z);
  fprintf(htmlfp, "      var original_camera_pos = camera_pos.slice(0);\n");
  fprintf(htmlfp, "      var original_center_pos = center_pos.slice(0);\n");
  fprintf(htmlfp, "      var original_up_dir = up_dir.slice(0);\n");
  fprintf(htmlfp, "      function drawScene() {\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        if (!viewMatrix) {\n");
  fprintf(htmlfp, "          calculateViewMatrix();\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        var verticalFieldOfView = %g;\n", context_struct_.vertical_field_of_view);
  fprintf(htmlfp, "        var zNear = %g;\n", context_struct_.zNear);
  fprintf(htmlfp, "        var zFar = %g;\n", context_struct_.zFar);
  fprintf(htmlfp, "        var aspect = 1.0*gl.viewportWidth/gl.viewportHeight;\n");
  fprintf(htmlfp, "        var f = 1/Math.tan(verticalFieldOfView*Math.PI/360.0);\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        var projectionMatrix = [\n");
  fprintf(htmlfp, "          f/aspect, 0.0, 0.0, 0.0,\n");
  fprintf(htmlfp, "          0.0, f, 0.0, 0.0,\n");
  fprintf(htmlfp, "          0.0, 0.0, (zFar+zNear)/(zNear-zFar), 2*zFar*zNear/(zNear-zFar),\n");
  fprintf(htmlfp, "          0.0, 0.0, -1, 0.0\n");
  fprintf(htmlfp, "        ];\n");
  fprintf(htmlfp, "        projectionMatrix = transposeMatrix4(projectionMatrix);\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        var lightDirection = [\n");
  fprintf(htmlfp, "          %g, %g, %g\n", context_struct_.light_dir[0], context_struct_.light_dir[1], context_struct_.light_dir[2]);
  fprintf(htmlfp, "        ];\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        gl.clearColor(%g,%g,%g,%g);\n", context_struct_.background_color[0], context_struct_.background_color[1], context_struct_.background_color[2], context_struct_.background_color[3]);
  fprintf(htmlfp, "        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);\n");
  {
    GR3_DrawList_t_ *draw;
    draw = context_struct_.draw_list_;
    while (draw) {
      GLfloat forward[3], up[3], left[3];
      GLfloat model_matrix[4][4] = {{0}};
      float tmp;
      fprintf(htmlfp, "        var meshId = %u;\n", draw->mesh);
      fprintf(htmlfp, "        var mesh = null;\n");
      fprintf(htmlfp, "        for (var meshIndex in meshes) {\n");
      fprintf(htmlfp, "          if (meshes[meshIndex].id == meshId) {\n");
      fprintf(htmlfp, "            mesh = meshes[meshIndex];\n");
      fprintf(htmlfp, "            break;\n");
      fprintf(htmlfp, "          }\n");
      fprintf(htmlfp, "        }\n");
      fprintf(htmlfp, "        mesh.bind()\n");
      fprintf(htmlfp, "        \n");
      fprintf(htmlfp, "        var modelMatrices = new Array();\n");
      fprintf(htmlfp, "        var scales = new Array();\n");
      fprintf(htmlfp, "        var colors = new Array();\n");
      for (i = 0; i < draw->n; i++) {
        {
          
          /* Calculate an orthonormal base in IR^3, correcting the up vector
           * in case it is not perpendicular to the forward vector. This base
           * is used to create the model matrix as a base-transformation
           * matrix.
           */
          /* forward = normalize(&directions[i*3]); */
          tmp = 0;
          for (j = 0; j < 3; j++) {
            tmp+= draw->directions[i*3+j]*draw->directions[i*3+j];
          }
          tmp = sqrt(tmp);
          for (j = 0; j < 3; j++) {
            forward[j] = draw->directions[i*3+j]/tmp;
          }/* up = normalize(&ups[i*3]); */
          tmp = 0;
          for (j = 0; j < 3; j++) {
            tmp+= draw->ups[i*3+j]*draw->ups[i*3+j];
          }
          tmp = sqrt(tmp);
          for (j = 0; j < 3; j++) {
            up[j] = draw->ups[i*3+j]/tmp;
          }
          /* left = cross(forward,up); */
          for (j = 0; j < 3; j++) {
            left[j] = forward[(j+1)%3]*up[(j+2)%3] - up[(j+1)%3]*forward[(j+2)%3];
          }/* left = normalize(left); */
          tmp = 0;
          for (j = 0; j < 3; j++) {
            tmp+= left[j]*left[j];
          }
          tmp = sqrt(tmp);
          for (j = 0; j < 3; j++) {
            left[j] = left[j]/tmp;
          }
          /* up = cross(left,forward); */
          for (j = 0; j < 3; j++) {
            up[j] = left[(j+1)%3]*forward[(j+2)%3] - forward[(j+1)%3]*left[(j+2)%3];
          }
          for (j = 0; j < 3; j++) {
            model_matrix[0][j] = -left[j];
            model_matrix[1][j] = up[j];
            model_matrix[2][j] = forward[j];
            model_matrix[3][j] = draw->positions[i*3+j];
          }
          model_matrix[3][3] = 1;
        }
        
        fprintf(htmlfp, "        var modelMatrix = [\n");
        fprintf(htmlfp, "          %g, %g, %g, %g,\n", model_matrix[0][0], model_matrix[1][0], model_matrix[2][0], model_matrix[3][0]);
        fprintf(htmlfp, "          %g, %g, %g, %g,\n", model_matrix[0][1], model_matrix[1][1], model_matrix[2][1], model_matrix[3][1]);
        fprintf(htmlfp, "          %g, %g, %g, %g,\n", model_matrix[0][2], model_matrix[1][2], model_matrix[2][2], model_matrix[3][2]);
        fprintf(htmlfp, "          %g, %g, %g, %g\n", model_matrix[0][3], model_matrix[1][3], model_matrix[2][3], model_matrix[3][3]);
        fprintf(htmlfp, "        ];\n");
        fprintf(htmlfp, "        modelMatrices.push(transposeMatrix4(modelMatrix));\n");
        
        fprintf(htmlfp, "        var scale = [\n");
        fprintf(htmlfp, "          %g, %g, %g\n", draw->scales[i*3+0], draw->scales[i*3+1], draw->scales[i*3+2]);
        fprintf(htmlfp, "        ];\n");
        fprintf(htmlfp, "        scales.push(scale);\n");
        fprintf(htmlfp, "        \n");
        fprintf(htmlfp, "        var color = [\n");
        fprintf(htmlfp, "          %g, %g, %g\n", draw->colors[i*3+0], draw->colors[i*3+1], draw->colors[i*3+2]);
        fprintf(htmlfp, "        ];\n");
        fprintf(htmlfp, "        colors.push(color);\n");
      }
      fprintf(htmlfp, "        gl.enable(gl.BLEND);\n");
      fprintf(htmlfp, "        gl.blendFunc(gl.CONSTANT_COLOR, gl.ZERO);\n");
      fprintf(htmlfp, "        for (var i = 0; i < %u; i++) {\n", draw->n);
      fprintf(htmlfp, "          gl.blendColor(colors[i][0],colors[i][1],colors[i][2],1.0);\n");
      fprintf(htmlfp, "          mesh.draw(projectionMatrix, viewMatrix, modelMatrices[i], scales[i], lightDirection);\n");
      fprintf(htmlfp, "        }\n");
      draw = draw->next;
    }
  }
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "      var shaderProgram;");
  fprintf(htmlfp, "      function initShaderProgram() {\n");
  fprintf(htmlfp, "        var vertexShader = getShader(gl, \"shader-vs\");\n");
  fprintf(htmlfp, "        var fragmentShader = getShader(gl, \"shader-fs\");\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        shaderProgram = gl.createProgram();\n");
  fprintf(htmlfp, "        gl.attachShader(shaderProgram, vertexShader);\n");
  fprintf(htmlfp, "        gl.attachShader(shaderProgram, fragmentShader);\n");
  fprintf(htmlfp, "        gl.linkProgram(shaderProgram);\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {\n");
  fprintf(htmlfp, "          alert(\"Unable to initialize the shader program.\");\n");
  fprintf(htmlfp, "          alert(gl.getProgramInfoLog(shaderProgram));\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        gl.useProgram(shaderProgram);\n");
  
  fprintf(htmlfp, "        shaderProgram.projectionMatrixLocation = gl.getUniformLocation(shaderProgram, \"ProjectionMatrix\");\n");
  fprintf(htmlfp, "        shaderProgram.viewMatrixLocation = gl.getUniformLocation(shaderProgram, \"ViewMatrix\");\n");
  fprintf(htmlfp, "        shaderProgram.modelMatrixLocation = gl.getUniformLocation(shaderProgram, \"ModelMatrix\");\n");
  fprintf(htmlfp, "        shaderProgram.lightDirectionLocation = gl.getUniformLocation(shaderProgram, \"LightDirection\");\n");
  fprintf(htmlfp, "        shaderProgram.scalesLocation = gl.getUniformLocation(shaderProgram, \"Scales\");\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        shaderProgram.vertexLocation = gl.getAttribLocation(shaderProgram, \"in_Vertex\");\n");
  fprintf(htmlfp, "        shaderProgram.normalLocation = gl.getAttribLocation(shaderProgram, \"in_Normal\");\n");
  fprintf(htmlfp, "        shaderProgram.colorLocation = gl.getAttribLocation(shaderProgram, \"in_Color\");\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "      function getShader(gl, id) {\n");
  fprintf(htmlfp, "        var shaderScriptElement = document.getElementById(id);\n");
  fprintf(htmlfp, "        if (!shaderScriptElement) {\n");
  fprintf(htmlfp, "          return null;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        var str = \"\";\n");
  fprintf(htmlfp, "        var k = shaderScriptElement.firstChild;\n");
  fprintf(htmlfp, "        while (k) {\n");
  fprintf(htmlfp, "          if (k.nodeType == 3) {\n");
  fprintf(htmlfp, "            str += k.textContent;\n");
  fprintf(htmlfp, "          }\n");
  fprintf(htmlfp, "          k = k.nextSibling;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        var shader;\n");
  fprintf(htmlfp, "        if (shaderScriptElement.type == \"x-shader/x-vertex\") {\n");
  fprintf(htmlfp, "          shader = gl.createShader(gl.VERTEX_SHADER);\n");
  fprintf(htmlfp, "        } else if (shaderScriptElement.type == \"x-shader/x-fragment\") {\n");
  fprintf(htmlfp, "          shader = gl.createShader(gl.FRAGMENT_SHADER);\n");
  fprintf(htmlfp, "        } else {\n");
  fprintf(htmlfp, "          return null;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        gl.shaderSource(shader, str);\n");
  fprintf(htmlfp, "        gl.compileShader(shader);\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {\n");
  fprintf(htmlfp, "          alert(gl.getShaderInfoLog(shader));\n");
  fprintf(htmlfp, "          return null;\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        \n");
  fprintf(htmlfp, "        return shader;\n");
  fprintf(htmlfp, "      }\n");
  
  fprintf(htmlfp, "    </script>\n");
  
  fprintf(htmlfp, "    <script id=\"shader-vs\" type=\"x-shader/x-vertex\">\n");
  fprintf(htmlfp, "      uniform mat4 ProjectionMatrix;\n");
  fprintf(htmlfp, "      uniform mat4 ViewMatrix;\n");
  fprintf(htmlfp, "      uniform mat4 ModelMatrix;\n");
  fprintf(htmlfp, "      uniform vec3 LightDirection;\n");
  fprintf(htmlfp, "      uniform vec3 Scales;\n");
  fprintf(htmlfp, "      attribute vec3 in_Vertex;\n");
  fprintf(htmlfp, "      attribute vec3 in_Normal;\n");
  fprintf(htmlfp, "      attribute vec3 in_Color;\n");
  fprintf(htmlfp, "      varying vec4 Color;\n");
  fprintf(htmlfp, "      varying vec3 Normal;\n");
  fprintf(htmlfp, "      void main(void) {\n");
  fprintf(htmlfp, "        vec4 Position = ViewMatrix*ModelMatrix*(vec4(Scales*in_Vertex,1));\n");
  fprintf(htmlfp, "        gl_Position=ProjectionMatrix*Position;\n");
  fprintf(htmlfp, "        Normal = vec3(ViewMatrix*ModelMatrix*vec4(in_Normal,0)).xyz;\n");
  fprintf(htmlfp, "        Color = vec4(in_Color,1);\n");
  fprintf(htmlfp, "        float diffuse = Normal.z;\n");
  fprintf(htmlfp, "        if (dot(LightDirection,LightDirection) > 0.001) {\n");
  fprintf(htmlfp, "          diffuse = dot(normalize(LightDirection),Normal);\n");
  fprintf(htmlfp, "        }\n");
  fprintf(htmlfp, "        Color.rgb = diffuse*Color.rgb;\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "    </script>\n");
  
  fprintf(htmlfp, "    <script id=\"shader-fs\" type=\"x-shader/x-fragment\">\n");
  fprintf(htmlfp, "      precision mediump float;\n");
  fprintf(htmlfp, "      varying vec4 Color;\n");
  fprintf(htmlfp, "      varying vec3 Normal;\n");
  fprintf(htmlfp, "      void main(void) {\n");
  fprintf(htmlfp, "        gl_FragColor=vec4(Color.rgb,Color.a);\n");
  fprintf(htmlfp, "      }\n");
  fprintf(htmlfp, "    </script>\n");
  
  fprintf(htmlfp, "  </head>\n");
  fprintf(htmlfp, "  <body onload=\"startWebGLCanvas()\">\n");
  fprintf(htmlfp, "    <canvas id=\"webgl-canvas\" width=\"%u\" height=\"%u\" tabindex=\"1\" style=\"outline-style:none;\"></canvas>\n", width, height);
  fprintf(htmlfp, "  </body>\n");
  fprintf(htmlfp, "</html>");
  fclose(htmlfp);
  return GR3_ERROR_NONE;
}
