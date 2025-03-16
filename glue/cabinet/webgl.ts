export function createCanvas(width = 800, height = 600) {
  const canvas = document.createElement('canvas');
  canvas.width = width;
  canvas.height = height;
  return canvas;
}

export function initWebGL(canvas: HTMLCanvasElement) {
  const gl = canvas.getContext('webgl2');
  if (!gl) {
    throw new Error('WebGL not supported');
  }
  return gl;
}

export function createShaderFromSrc(gl: WebGL2RenderingContext, type: number, src: string) {
  const shader = gl.createShader(type);
  if (!shader) {
    throw new Error('Failed to create shader');
  }
  gl.shaderSource(shader, src);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    throw new Error('Failed to compile shader: ' + gl.getShaderInfoLog(shader));
  }
  return shader;
}

export function createProgramFromSrc(gl: WebGL2RenderingContext, vtxSrc: string, fragSrc: string) {
  const program = gl.createProgram();
  if (!program) {
    throw new Error('Failed to create program');
  }
  const vtxShader = createShaderFromSrc(gl, gl.VERTEX_SHADER, vtxSrc);
  const fragShader = createShaderFromSrc(gl, gl.FRAGMENT_SHADER, fragSrc);
  gl.attachShader(program, vtxShader);
  gl.attachShader(program, fragShader);
  gl.linkProgram(program);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    throw new Error('Failed to link program: ' + gl.getProgramInfoLog(program));
  }
  return program;
}

export function createCrispTexture(gl: WebGL2RenderingContext, img: HTMLImageElement) {
  const texture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, img);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  console.log('Created texture', texture);
  return texture;
}

export function createCrispTextureArray(gl: WebGL2RenderingContext, img: HTMLImageElement, count: number) {
  const tempCanvas = document.createElement('canvas');
  tempCanvas.width = img.width;
  tempCanvas.height = img.height; // Combined height
  const width = img.width;
  const height = img.height / count;
  const tempCtx = tempCanvas.getContext('2d');
  if (!tempCtx) throw new Error("2D context creation failed on temp canvas");
  tempCtx.drawImage(img, 0, 0);
  const combinedImageData = tempCtx.getImageData(0, 0, img.width, img.height);
  const combinedData = new Uint8Array(combinedImageData.data);

  const texture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D_ARRAY, texture);
  gl.texImage3D(gl.TEXTURE_2D_ARRAY, 0, gl.RGBA, width, height, count, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
  for (let i = 0; i < count; i++) {
    const offset = i * width * height * 4;
    gl.texSubImage3D(gl.TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, gl.RGBA, gl.UNSIGNED_BYTE, combinedData.subarray(offset, offset + width * height * 4));
  }
  gl.texParameteri(gl.TEXTURE_2D_ARRAY, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D_ARRAY, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.generateMipmap(gl.TEXTURE_2D_ARRAY);
  console.log('Created texture array', texture);
  return texture;
}

