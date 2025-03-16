import { AttributeCfg, Cabinet, Geometry, GeometryCfg, Gfx, Shader, Texture } from "./cabinet/cabinet";
import wasmUrl from "./wasm/main.wasm?url";

interface WasmExports {
  init(): void;
  update(): void;
  shutdown(): void;
}

type StoreObject = Cabinet | Gfx | Shader | Geometry | HTMLImageElement | Texture | WebGLTexture | WebGLBuffer;
let idCounter = 0;
const objectStore = new Map<number, StoreObject>();

function storeObject(obj: StoreObject): number {
  const id = idCounter++;
  objectStore.set(id, obj);
  return id;
}

function getObject(id: number): StoreObject | undefined {
  return objectStore.get(id);
}

function removeObject(id: number): void {
  objectStore.delete(id);
}

// Type guards for type safety
function isCabinet(obj?: StoreObject): obj is Cabinet {
  return obj instanceof Cabinet;
}

function isGfx(obj?: StoreObject): obj is Gfx {
  return obj instanceof Gfx;
}

function isShader(obj?: StoreObject): obj is Shader {
  return obj instanceof Shader;
}

function isGeometry(obj?: StoreObject): obj is Geometry {
  return obj instanceof Geometry;
}

function getShader(id: number): Shader | undefined {
  const obj = getObject(id);
  if (isShader(obj)) {
    return obj;
  }
  return undefined;
}

function decodeCString(memory: WebAssembly.Memory, ptr: number): string {
  const bytes = new Uint8Array(memory.buffer);
  const nullIdx = bytes.indexOf(0, ptr);
  if (nullIdx === -1) {
    throw new Error("CString not null-terminated");
  }
  const slice = bytes.slice(ptr, nullIdx);
  return new TextDecoder().decode(slice);
}

function getAttributeCfg(memory: WebAssembly.Memory, ptr: number): AttributeCfg {
  const view = new DataView(memory.buffer);
  const namePtr = view.getUint32(ptr, true);
  const buffer = view.getUint32(ptr + 4, true);
  const size = view.getUint32(ptr + 8, true);
  const type = view.getUint32(ptr + 12, true);
  const stride = view.getUint32(ptr + 16, true);
  const offset = view.getUint32(ptr + 20, true);
  const name = decodeCString(memory, namePtr);
  return { name, buffer, size, type, stride, offset };
}

function getFloat32Array(memory: WebAssembly.Memory, ptr: number, len: number): Float32Array {
  return new Float32Array(memory.buffer, ptr, len);
}

function getGeometryCfg(memory: WebAssembly.Memory, ptr: number): GeometryCfg {
  const view = new DataView(memory.buffer);
  const buffersPtr = view.getUint32(ptr, true);
  const bufferCount = view.getUint32(ptr + 4, true);
  const attributesPtr = view.getUint32(ptr + 8, true);
  const attributeCount = view.getUint32(ptr + 12, true);
  const vertexCount = view.getUint32(ptr + 16, true);
  const mode = view.getUint32(ptr + 20, true);

  const buffers = [];
  const attributes = [];
  let bufferIdx = buffersPtr;
  for (let i = 0; i < bufferCount; i++) {
    const bufferLen = view.getUint32(bufferIdx + 4, true);
    const dataPtr = view.getUint32(bufferIdx, true);
    const buffer = getFloat32Array(memory, dataPtr, bufferLen / 4);
    buffers.push(buffer);
    bufferIdx += 4 + bufferLen * 4;
  }

  let attributeIdx = attributesPtr;
  for (let i = 0; i < attributeCount; i++) {
    const attribute = getAttributeCfg(memory, attributeIdx);
    attributes.push(attribute);
    attributeIdx += 24;
  }

  return { buffers, attributes, vertexCount, mode };
}

async function init() {
  const wasmModule = await WebAssembly.instantiateStreaming(
    fetch(wasmUrl), {
    env: {
      cabinet_create() {
        const cabinet = new Cabinet();
        return storeObject(cabinet);
      },
      cabinet_destroy(id: number) {
        const obj = getObject(id);
        if (isCabinet(obj)) {
          obj.destroy();
        }
        removeObject(id);
      },
      cabinet_load_image(id: number, ptr: number) {
        const cabinet = getObject(id);
        const path = decodeCString(wasmModule.instance.exports.memory as any, ptr);
        if (isCabinet(cabinet)) {
          const img = cabinet.loadImage(path);
          return storeObject(img);
        }
      },
      cabinet_is_loaded(id: number, handle: number) {
        const cabinet = getObject(id);
        const img = getObject(handle);
        if (img instanceof HTMLImageElement) {
          console.log("Checking if image is loaded", img.complete);
          return img.complete;
        }
        return false;
      },
      gfx_create(id: number) {
        const cabinet = getObject(id);
        if (isCabinet(cabinet)) {
          const gfx = cabinet.createGfx();
          return storeObject(gfx);
        }
        return -1;
      },
      gfx_destroy(id: number) {
        const obj = getObject(id);
        if (isGfx(obj)) {
          obj.destroy();
        }
        removeObject(id);
      },
      gfx_clear(id: number, r: number, g: number, b: number, a: number) {
        const obj = getObject(id);
        if (isGfx(obj)) {
          obj.clear(r, g, b, a);
        } else {
          console.error(`gfx_clear: object with id ${id} is not a Gfx`);
        }
      },
      gfx_create_shader(id: number, vtxSrcPtr: number, fragSrcPtr: number) {
        const obj = getObject(id);
        if (isGfx(obj)) {
          const vtxSrc = decodeCString(wasmModule.instance.exports.memory as any, vtxSrcPtr);
          const fragSrc = decodeCString(wasmModule.instance.exports.memory as any, fragSrcPtr);
          const shader = obj.createShader(vtxSrc, fragSrc);
          return storeObject(shader);
        }
        return -1;
      },
      gfx_bind_shader(id: number, shaderId: number) {
        const obj = getObject(id);
        const shader = getShader(shaderId);
        if (isGfx(obj) && shader) {
          obj.bindShader(shader);
        }
      },
      gfx_create_uniforms(id: number, ptr: number, size: number) {
        const gfx = getObject(id);
        const name = decodeCString(wasmModule.instance.exports.memory as any, ptr);
        if (isGfx(gfx)) {
          const uniforms = gfx.createUniforms(name, size);
          return storeObject(uniforms);
        }
        return -1;
      },
      gfx_update_uniforms(id: number, uniformsId: number, ptr: number, len: number) {
        const gfx = getObject(id);
        const uniforms = getObject(uniformsId);
        if (isGfx(gfx) && uniforms instanceof WebGLBuffer) {
          const data = new Uint8Array((wasmModule.instance.exports.memory as any).buffer, ptr, len);
          gfx.updateUniforms(uniforms, data);
        }
      },
      gfx_create_geometry(id: number, ptr: number) {
        const gfx = getObject(id);
        const cfg = getGeometryCfg(wasmModule.instance.exports.memory as any, ptr);
        console.log("Geometry config", cfg);

        if (isGfx(gfx)) {
          const geometry = gfx.createGeometry(cfg);
          return storeObject(geometry);
        }
        return -1;
      },
      gfx_draw_geometry(id: number, geometryId: number) {
        const gfx = getObject(id);
        if (!isGfx(gfx)) return;

        const geometry = getObject(geometryId);
        if (!isGeometry(geometry)) return;

        const gl = gfx.gl;
        gl.bindVertexArray(geometry.vao);
        gl.drawArrays(gl.TRIANGLES, 0, geometry.vertexCount);
      },
        gfx_set_vertex_count(geometryId: number, vertexCount: number) {
          const geometry = getObject(geometryId);
          if (!isGeometry(geometry)) return;
          geometry.vertexCount = vertexCount;
        },
        gfx_update_geometry(gfxId: number, geometryId: number, bufferIdx: number, ptr: number, size: number) {
          const gfx = getObject(gfxId);
          if (!isGfx(gfx)) return;
          const geometry = getObject(geometryId);
          if (!isGeometry(geometry)) return;
          const gl = gfx.gl;
          const buffer = geometry.buffers[bufferIdx];
          const data = new Float32Array((wasmModule.instance.exports.memory as any).buffer, ptr, size);
          gl.bindVertexArray(geometry.vao);
          gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
          gl.bufferSubData(gl.ARRAY_BUFFER, 0, data, 0, size);
        }, 
      gfx_create_texture(id: number, imgId: number) {
        const gfx = getObject(id);
        if (!isGfx(gfx)) return;
        const img = getObject(imgId);
        if (!(img instanceof HTMLImageElement)) return;
        const texture = gfx.createTexture(img);
        return storeObject(texture);
      },
        gfx_create_texture_array(id: number, imgId: number, imgCount: number) {
          const gfx = getObject(id);
          if (!isGfx(gfx)) return;
          const img = getObject(imgId);
          if (!(img instanceof HTMLImageElement)) return;
          const texture = gfx.createTextureArray(img, imgCount);
          return storeObject(texture);
        },
      gfx_bind_texture(id: number, textureId: number) {
        const gfx = getObject(id);
        if (!isGfx(gfx)) return;
        const texture = getObject(textureId) as Texture;
        const gl = gfx.gl;
        gl.bindTexture(texture.isArray ? gl.TEXTURE_2D_ARRAY : gl.TEXTURE_2D, texture.texture);
      },
      shader_destroy(id: number) {
        const obj = getObject(id);
        if (isShader(obj)) {
          obj.destroy();
        }
        removeObject(id);
      },
      log_info(cab: number, ptr: number) {
        console.log(decodeCString(wasmModule.instance.exports.memory as any, ptr));
      },
      cab_sin(x: number) {
        return Math.sin(x);
      },
      cab_cos(x: number) {
        return Math.cos(x);
      },
      cab_tan(x: number) {
        return Math.tan(x);
      },
      cab_acos(x: number) {
        return Math.acos(x);
      },
      cab_sqrt(x: number) {
        return Math.sqrt(x);
      },
    }
  }
  );
  const exports = wasmModule.instance.exports as any as WasmExports;
  console.log("Wasm exports:", exports);

  let running = true;
  exports.init();
  function loop() {
    if (running) {
      exports.update();
      requestAnimationFrame(loop);
    } else {
      exports.shutdown();
    }
  }
  requestAnimationFrame(loop);
}

init();
