import { createCanvas, createCrispTexture, createCrispTextureArray, createProgramFromSrc, initWebGL } from "./webgl";

export class Shader {
  program: WebGLProgram;

  constructor(program: WebGLProgram) {
    this.program = program;
  }

  destroy() {
  }
}

export class Cabinet {
  loadImage(path: string) {
    const img = new Image();
    img.onload = () => {
    }
    img.onerror = () => {
    }
    img.src = path;
    return img;
  }

  constructor() {
  }

  createGfx() {
    return new Gfx(this);
  }

  destroy() {
  }

}

export interface UniformCfg {
  name: string;
  type: number;
  size: number;
}

export interface UniformsCfg {
  uniforms: UniformCfg[];
}

export interface AttributeCfg {
  name: string;
  buffer: number;
  size: number;
  type: number;
  stride: number;
  offset: number;
}

export interface GeometryCfg {
  buffers: Array<Float32Array>;
  attributes: AttributeCfg[];
  vertexCount: number;
  mode: number;
}

export class Geometry {
  vao: WebGLVertexArrayObject;
  buffers: WebGLBuffer[];
  vertexCount: number;

  constructor(vao: WebGLVertexArrayObject, vbos: WebGLBuffer[], vertexCount: number) {
    this.vao = vao;
    this.buffers = vbos;
    this.vertexCount = vertexCount;
  }

  destroy() {
  }
}

export interface Texture {
  texture: WebGLTexture;
  isArray: boolean;
}

export class Gfx {
  cabinet: Cabinet;
  canvas: HTMLCanvasElement;
  gl: WebGL2RenderingContext;

  constructor(cabinet: Cabinet) {
    this.cabinet = cabinet;
    this.canvas = createCanvas(1920, 1080); 
    document.body.appendChild(this.canvas);
    this.gl = initWebGL(this.canvas);
    const gl = this.gl;
    gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    gl.enable(gl.DEPTH_TEST);
    gl.disable(gl.CULL_FACE);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
  }

  clear(r: number, g: number, b: number, a: number) {
    this.gl.clearColor(r, g, b, a);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
  }

  createShader(vtxSrc: string, fragSrc: string) {
    const program = createProgramFromSrc(this.gl, vtxSrc, fragSrc);
    return new Shader(program);
  }

  bindShader(shader: Shader) {
    this.gl.useProgram(shader.program);
  }

  createGeometry(cfg: GeometryCfg) {
    const vao = this.gl.createVertexArray();

    const program = this.gl.getParameter(this.gl.CURRENT_PROGRAM);
    if (!vao) {
      throw new Error('Failed to create vertex array');
    }
    this.gl.bindVertexArray(vao);

    const vbo = this.gl.createBuffer();
    if (!vbo) {
      throw new Error('Failed to create buffer');
    }
    this.gl.bindBuffer(this.gl.ARRAY_BUFFER, vbo);
    this.gl.bufferData(this.gl.ARRAY_BUFFER,
      cfg.buffers[0],
      this.gl.STATIC_DRAW);

    for (let i = 0; i < cfg.attributes.length; i++) {
      const attr = cfg.attributes[i];
      const location = this.gl.getAttribLocation(program, attr.name);
      this.gl.enableVertexAttribArray(location);
      const type = this.gl.FLOAT;
      this.gl.vertexAttribPointer(location, attr.size, type, false, attr.stride, attr.offset);
    }

    return new Geometry(vao, [vbo], cfg.vertexCount);
  }

  createUniforms(name: string, size: number) {
    const gl = this.gl;
    const program = gl.getParameter(gl.CURRENT_PROGRAM);
    const location = gl.getUniformBlockIndex(program, name);
    console.log("Uniform block index", location);
    gl.uniformBlockBinding(program, location, 0);
    const minSize = gl.getParameter(gl.UNIFORM_BLOCK_DATA_SIZE)
    const data = new Float32Array(Math.max(size, minSize));
    data.fill(0);
    data.set([
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1,
    ]);
    const buffer = gl.createBuffer();
    gl.bindBuffer(gl.UNIFORM_BUFFER, buffer);
    gl.bufferData(gl.UNIFORM_BUFFER, data, gl.STATIC_DRAW);
    gl.bindBufferBase(gl.UNIFORM_BUFFER, 0, buffer);

    return buffer;
  }

  updateUniforms(buffer: WebGLBuffer, data: ArrayBufferLike) {
    const gl = this.gl;
    gl.bindBuffer(gl.UNIFORM_BUFFER, buffer);
    gl.bufferSubData(gl.UNIFORM_BUFFER, 0, data);
  }

  createTexture(img: HTMLImageElement): Texture {
    return {
      texture: createCrispTexture(this.gl, img),
      isArray: false,
    };
  }

  createTextureArray(img: HTMLImageElement, count: number): Texture {
    return {
      texture: createCrispTextureArray(this.gl, img, count),
      isArray: true,
    };
  }

  destroy() {
  }

}
