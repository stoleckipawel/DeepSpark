/* hero-particles.js - Sponza Palace ray-tracing visualisation
   Two-story arcade with semicircular arches, balustrade,
   open-air roof, and depth-of-field defocused ray bounces. */

(function () {
  'use strict';

  var canvas = document.getElementById('hero-particles');
  if (!canvas) return;
  var ctx = canvas.getContext('2d');
  var width, height, dpr, raf;
  var SCENE_ASPECT = 2.8;  /* reference aspect ratio (wide corridor) */
  var sceneW, sceneH, offX, offY;

  /* -- Tunables ------------------------------------------------- */
  var RAY_COUNT      = 22;
  var RAY_LIFETIME   = 16000;
  var SPAWN_STAGGER  = 600;
  var MAX_BOUNCES    = 8;
  var FOCAL_DEPTH    = 0.35;
  var DOF_STRENGTH   = 16;
  var SCENE_ALPHA    = 0.20;
  var TAIL_FRACTION  = 0.38;

  /* Warm palette for rays */
  var palette = [
    [255, 159, 28],   // Thread Orange
    [255, 209, 102],  // Ray Gold
    [232, 114, 12],   // Burnt Orange
    [255, 228, 160],  // Warm highlight
    [220, 160, 80],   // Sandy
  ];

  /* Architecture colour */
  var stoneCol = [210, 185, 140];

  /* Warm white for ray hot core */
  var warmWhite = [255, 240, 210];

  var rays = [];
  var sceneSegs = [];
  var nextSpawnTime = 0;

  function rand(a, b) { return a + Math.random() * (b - a); }
  function pick(arr) { return arr[Math.floor(Math.random() * arr.length)]; }
  function clamp01(v) { return v < 0 ? 0 : v > 1 ? 1 : v; }

  function cocRadius(depth) {
    return 3.0 + Math.abs(depth - FOCAL_DEPTH) * DOF_STRENGTH;
  }

  /* -- Perspective projection helpers --------------------------- */
  var VPX = 0.50, VPY = 0.36;

  function perspX(x, z) { return VPX + (x - VPX) * (1 - z * 0.88); }
  function perspY(y, z) { return VPY + (y - VPY) * (1 - z * 0.88); }

  /* -- Build Sponza scene geometry ------------------------------ */
  function buildScene() {
    sceneSegs = [];

    function seg(x0, y0, x1, y1, depth, col) {
      sceneSegs.push({ x0: x0, y0: y0, x1: x1, y1: y1, depth: depth, col: col || null });
    }

    /* 5 column positions = 4 arcade bays (cleaner, less noisy) */
    var depths = [0.0, 0.18, 0.38, 0.58, 0.80];

    /* Vertical layout (y = 0 top, y = 1 bottom) */
    var floorY   = 0.96;    // floor pushed very low
    var colCapY  = 0.54;    // ground-floor column capital / arch springing
    var galleryY = 0.36;    // gallery floor ~ GF arch crown
    var balTopY  = 0.34;    // balustrade top rail
    var uColCapY = 0.16;    // upper-gallery column capital
    var corniceY = 0.06;    // cornice above upper arches

    /* Horizontal layout - NARROWER corridor like real Sponza */
    var wallL = 0.04,  wallR = 0.96;
    var colLo = 0.14,  colRo = 0.86;  // column outer face
    var colLi = 0.26,  colRi = 0.74;  // column inner face

    var gfArchRise = colCapY - galleryY;   // 0.18
    var ugArchRise = uColCapY - corniceY;  // 0.10

    for (var i = 0; i < depths.length; i++) {
      var z  = depths[i];
      var zn = i < depths.length - 1 ? depths[i + 1] : z + 0.14;
      var zm = (z + zn) * 0.5;

      /* -- Floor ------------------------------------------------ */
      seg(perspX(colLi, z), perspY(floorY, z),
          perspX(colRi, z), perspY(floorY, z), z);
      if (i < depths.length - 1) {
        seg(perspX(colLi, z), perspY(floorY, z),
            perspX(colLi, zn), perspY(floorY, zn), zm);
        seg(perspX(colRi, z), perspY(floorY, z),
            perspX(colRi, zn), perspY(floorY, zn), zm);
      }

      /* -- Ground-floor columns (both faces) -------------------- */
      /* Left */
      seg(perspX(colLo, z), perspY(floorY, z),
          perspX(colLo, z), perspY(colCapY, z), z);
      seg(perspX(colLi, z), perspY(floorY, z),
          perspX(colLi, z), perspY(colCapY, z), z);
      seg(perspX(colLo, z), perspY(colCapY, z),
          perspX(colLi, z), perspY(colCapY, z), z);
      seg(perspX(colLo, z), perspY(floorY, z),
          perspX(colLi, z), perspY(floorY, z), z);
      /* Right */
      seg(perspX(colRi, z), perspY(floorY, z),
          perspX(colRi, z), perspY(colCapY, z), z);
      seg(perspX(colRo, z), perspY(floorY, z),
          perspX(colRo, z), perspY(colCapY, z), z);
      seg(perspX(colRi, z), perspY(colCapY, z),
          perspX(colRo, z), perspY(colCapY, z), z);
      seg(perspX(colRi, z), perspY(floorY, z),
          perspX(colRo, z), perspY(floorY, z), z);

      /* -- Ground-floor semicircular arches ---------------------- */
      if (i < depths.length - 1) {
        var aN = 10;
        var a, t0, t1, za, zb, ya, yb;
        /* Left arcade arch */
        for (a = 0; a < aN; a++) {
          t0 = a / aN; t1 = (a + 1) / aN;
          za = z + (zn - z) * t0; zb = z + (zn - z) * t1;
          ya = colCapY - gfArchRise * Math.sin(Math.PI * t0);
          yb = colCapY - gfArchRise * Math.sin(Math.PI * t1);
          seg(perspX(colLi, za), perspY(ya, za),
              perspX(colLi, zb), perspY(yb, zb), zm);
        }
        /* Right arcade arch */
        for (a = 0; a < aN; a++) {
          t0 = a / aN; t1 = (a + 1) / aN;
          za = z + (zn - z) * t0; zb = z + (zn - z) * t1;
          ya = colCapY - gfArchRise * Math.sin(Math.PI * t0);
          yb = colCapY - gfArchRise * Math.sin(Math.PI * t1);
          seg(perspX(colRi, za), perspY(ya, za),
              perspX(colRi, zb), perspY(yb, zb), zm);
        }
        /* Entablature / horizontal beams along z */
        seg(perspX(colLo, z), perspY(colCapY, z),
            perspX(colLo, zn), perspY(colCapY, zn), zm);
        seg(perspX(colLi, z), perspY(colCapY, z),
            perspX(colLi, zn), perspY(colCapY, zn), zm);
        seg(perspX(colRi, z), perspY(colCapY, z),
            perspX(colRi, zn), perspY(colCapY, zn), zm);
        seg(perspX(colRo, z), perspY(colCapY, z),
            perspX(colRo, zn), perspY(colCapY, zn), zm);
      }

      /* -- Gallery / mezzanine floor ---------------------------- */
      seg(perspX(wallL, z), perspY(galleryY, z),
          perspX(colLo, z), perspY(galleryY, z), z);
      seg(perspX(colRo, z), perspY(galleryY, z),
          perspX(wallR, z), perspY(galleryY, z), z);
      if (i < depths.length - 1) {
        seg(perspX(wallL, z), perspY(galleryY, z),
            perspX(wallL, zn), perspY(galleryY, zn), zm);
        seg(perspX(wallR, z), perspY(galleryY, z),
            perspX(wallR, zn), perspY(galleryY, zn), zm);
        seg(perspX(colLo, z), perspY(galleryY, z),
            perspX(colLo, zn), perspY(galleryY, zn), zm);
        seg(perspX(colRo, z), perspY(galleryY, z),
            perspX(colRo, zn), perspY(galleryY, zn), zm);
      }

      /* -- Balustrade ------------------------------------------- */
      seg(perspX(colLo, z), perspY(balTopY, z),
          perspX(colLo, z), perspY(galleryY, z), z);
      seg(perspX(colRo, z), perspY(balTopY, z),
          perspX(colRo, z), perspY(galleryY, z), z);
      seg(perspX(colLo, z), perspY(balTopY, z),
          perspX(colLi, z), perspY(balTopY, z), z);
      seg(perspX(colRi, z), perspY(balTopY, z),
          perspX(colRo, z), perspY(balTopY, z), z);
      if (i < depths.length - 1) {
        seg(perspX(colLo, z), perspY(balTopY, z),
            perspX(colLo, zn), perspY(balTopY, zn), zm);
        seg(perspX(colRo, z), perspY(balTopY, z),
            perspX(colRo, zn), perspY(balTopY, zn), zm);
      }

      /* -- Upper-gallery columns -------------------------------- */
      var ucLo = colLo + 0.01, ucLi = colLi - 0.01;
      var ucRi = colRi + 0.01, ucRo = colRo - 0.01;
      seg(perspX(ucLo, z), perspY(galleryY, z),
          perspX(ucLo, z), perspY(uColCapY, z), z);
      seg(perspX(ucLi, z), perspY(galleryY, z),
          perspX(ucLi, z), perspY(uColCapY, z), z);
      seg(perspX(ucLo, z), perspY(uColCapY, z),
          perspX(ucLi, z), perspY(uColCapY, z), z);
      seg(perspX(ucRi, z), perspY(galleryY, z),
          perspX(ucRi, z), perspY(uColCapY, z), z);
      seg(perspX(ucRo, z), perspY(galleryY, z),
          perspX(ucRo, z), perspY(uColCapY, z), z);
      seg(perspX(ucRi, z), perspY(uColCapY, z),
          perspX(ucRo, z), perspY(uColCapY, z), z);

      /* -- Upper-gallery arches --------------------------------- */
      if (i < depths.length - 1) {
        var uN = 6;
        for (a = 0; a < uN; a++) {
          t0 = a / uN; t1 = (a + 1) / uN;
          za = z + (zn - z) * t0; zb = z + (zn - z) * t1;
          ya = uColCapY - ugArchRise * Math.sin(Math.PI * t0);
          yb = uColCapY - ugArchRise * Math.sin(Math.PI * t1);
          seg(perspX(ucLi, za), perspY(ya, za),
              perspX(ucLi, zb), perspY(yb, zb), zm);
        }
        for (a = 0; a < uN; a++) {
          t0 = a / uN; t1 = (a + 1) / uN;
          za = z + (zn - z) * t0; zb = z + (zn - z) * t1;
          ya = uColCapY - ugArchRise * Math.sin(Math.PI * t0);
          yb = uColCapY - ugArchRise * Math.sin(Math.PI * t1);
          seg(perspX(ucRi, za), perspY(ya, za),
              perspX(ucRi, zb), perspY(yb, zb), zm);
        }
        seg(perspX(ucLi, z), perspY(uColCapY, z),
            perspX(ucLi, zn), perspY(uColCapY, zn), zm);
        seg(perspX(ucRi, z), perspY(uColCapY, z),
            perspX(ucRi, zn), perspY(uColCapY, zn), zm);
      }

      /* -- Cornice / roof --------------------------------------- */
      seg(perspX(wallL, z), perspY(corniceY, z),
          perspX(colLo, z), perspY(corniceY, z), z);
      seg(perspX(colRo, z), perspY(corniceY, z),
          perspX(wallR, z), perspY(corniceY, z), z);
      if (i < depths.length - 1) {
        seg(perspX(wallL, z), perspY(corniceY, z),
            perspX(wallL, zn), perspY(corniceY, zn), zm);
        seg(perspX(wallR, z), perspY(corniceY, z),
            perspX(wallR, zn), perspY(corniceY, zn), zm);
      }
      /* Open sky - no roof parapet (rays enter freely from above) */

      /* -- Outer walls ------------------------------------------ */
      seg(perspX(wallL, z), perspY(floorY, z),
          perspX(wallL, z), perspY(corniceY, z), z);
      seg(perspX(wallR, z), perspY(floorY, z),
          perspX(wallR, z), perspY(corniceY, z), z);
      if (i < depths.length - 1) {
        seg(perspX(wallL, z), perspY(floorY, z),
            perspX(wallL, zn), perspY(floorY, zn), zm);
        seg(perspX(wallR, z), perspY(floorY, z),
            perspX(wallR, zn), perspY(floorY, zn), zm);
      }


    }

    /* -- Back wall ---------------------------------------------- */
    var bz = depths[depths.length - 1];
    seg(perspX(wallL, bz), perspY(corniceY, bz),
        perspX(wallL, bz), perspY(floorY, bz), bz);
    seg(perspX(wallR, bz), perspY(corniceY, bz),
        perspX(wallR, bz), perspY(floorY, bz), bz);
    seg(perspX(wallL, bz), perspY(floorY, bz),
        perspX(wallR, bz), perspY(floorY, bz), bz);
    seg(perspX(wallL, bz), perspY(corniceY, bz),
        perspX(wallR, bz), perspY(corniceY, bz), bz);
    seg(perspX(wallL, bz), perspY(galleryY, bz),
        perspX(wallR, bz), perspY(galleryY, bz), bz);
    seg(perspX(wallL, bz), perspY(colCapY, bz),
        perspX(wallR, bz), perspY(colCapY, bz), bz);

    /* Back wall - large central arch */
    var baL = 0.30, baR = 0.70;
    var baBase = colCapY, baCrown = galleryY - 0.06;
    for (var a = 0; a < 14; a++) {
      var t0 = a / 14, t1 = (a + 1) / 14;
      var x0 = baL + (baR - baL) * t0, x1 = baL + (baR - baL) * t1;
      var y0 = baBase - (baBase - baCrown) * Math.sin(Math.PI * t0);
      var y1 = baBase - (baBase - baCrown) * Math.sin(Math.PI * t1);
      seg(perspX(x0, bz), perspY(y0, bz),
          perspX(x1, bz), perspY(y1, bz), bz);
    }
    seg(perspX(baL, bz), perspY(baBase, bz),
        perspX(baL, bz), perspY(floorY, bz), bz);
    seg(perspX(baR, bz), perspY(baBase, bz),
        perspX(baR, bz), perspY(floorY, bz), bz);

    /* Back wall - two smaller upper arches */
    var ubaBase = uColCapY, ubaCrown = corniceY + 0.02;
    var pairs = [[wallL + 0.03, 0.44], [0.56, wallR - 0.03]];
    for (var p = 0; p < pairs.length; p++) {
      var aL = pairs[p][0], aR = pairs[p][1];
      for (a = 0; a < 8; a++) {
        t0 = a / 8; t1 = (a + 1) / 8;
        x0 = aL + (aR - aL) * t0; x1 = aL + (aR - aL) * t1;
        y0 = ubaBase - (ubaBase - ubaCrown) * Math.sin(Math.PI * t0);
        y1 = ubaBase - (ubaBase - ubaCrown) * Math.sin(Math.PI * t1);
        seg(perspX(x0, bz), perspY(y0, bz),
            perspX(x1, bz), perspY(y1, bz), bz);
      }
    }

    /* Sort back-to-front + precompute per-segment invariants */
    sceneSegs.sort(function(a, b) { return b.depth - a.depth; });
    for (var pi = 0; pi < sceneSegs.length; pi++) {
      var ps = sceneSegs[pi];
      ps.depthOcc = Math.exp(-ps.depth * 2.4);
      ps.coc = cocRadius(ps.depth);
      ps.baseLw = Math.max(0.4, ps.coc * 0.18 * ps.depthOcc + 0.5);
      ps.baseCol = ps.col || stoneCol;
      ps.isColored = !!ps.col;
    }
  }

  /* -- Ray-segment intersection (2D) ---------------------------- */
  function intersectScene(ox, oy, dx, dy) {
    var bestT = 1e9, bestNx = 0, bestNy = 0, bestDepth = 0, hit = false;
    for (var si = 0; si < sceneSegs.length; si++) {
      var s = sceneSegs[si];
      var ex = s.x1 - s.x0, ey = s.y1 - s.y0;
      var denom = dx * ey - dy * ex;
      if (Math.abs(denom) < 1e-10) continue;
      var t = ((s.x0 - ox) * ey - (s.y0 - oy) * ex) / denom;
      var u = ((s.x0 - ox) * dy - (s.y0 - oy) * dx) / denom;
      if (t < 0.003 || u < 0 || u > 1 || t >= bestT) continue;
      bestT = t;
      var nx = -ey, ny = ex;
      var len = Math.sqrt(nx * nx + ny * ny) || 1;
      nx /= len; ny /= len;
      if (nx * dx + ny * dy > 0) { nx = -nx; ny = -ny; }
      bestNx = nx; bestNy = ny;
      bestDepth = s.depth;
      hit = true;
    }
    return hit ? { t: bestT, nx: bestNx, ny: bestNy, depth: bestDepth } : null;
  }

  /* -- Build ray by tracing ------------------------------------- */
  function buildRay(time) {
    var c = pick(palette);
    var ox, oy, angle;
    var r = Math.random();

    /* Spawn from random positions throughout the entire scene */
    if (r < 0.30) {
      /* Random position in the corridor floor area */
      ox = rand(0.28, 0.72);
      oy = rand(0.55, 0.90);
      angle = rand(0, 360) * Math.PI / 180;
    } else if (r < 0.50) {
      /* Random position in the open-air gap (sunlight entry) */
      ox = rand(0.30, 0.70);
      oy = rand(0.02, 0.30);
      angle = rand(0, 360) * Math.PI / 180;
    } else if (r < 0.65) {
      /* Near left columns */
      ox = rand(0.15, 0.30);
      oy = rand(0.20, 0.80);
      angle = rand(0, 360) * Math.PI / 180;
    } else if (r < 0.80) {
      /* Near right columns */
      ox = rand(0.70, 0.85);
      oy = rand(0.20, 0.80);
      angle = rand(0, 360) * Math.PI / 180;
    } else if (r < 0.90) {
      /* Gallery / balustrade level */
      ox = rand(0.20, 0.80);
      oy = rand(0.30, 0.45);
      angle = rand(0, 360) * Math.PI / 180;
    } else {
      /* Near back wall / deep corridor */
      ox = rand(0.35, 0.65);
      oy = rand(0.36, 0.50);
      angle = rand(0, 360) * Math.PI / 180;
    }

    var dir = { x: Math.cos(angle), y: Math.sin(angle) };
    var segments = [];
    var cx = ox, cy = oy;

    for (var b = 0; b <= MAX_BOUNCES; b++) {
      var hit = intersectScene(cx, cy, dir.x, dir.y);
      if (!hit || hit.t > 2.0) {
        var esc = hit ? Math.min(hit.t, 0.4) : 0.3;
        segments.push({
          x0: cx, y0: cy,
          x1: cx + dir.x * esc, y1: cy + dir.y * esc,
          depth: hit ? hit.depth : 0.5
        });
        break;
      }
      segments.push({
        x0: cx, y0: cy,
        x1: cx + dir.x * hit.t, y1: cy + dir.y * hit.t,
        depth: hit.depth
      });
      cx += dir.x * hit.t;
      cy += dir.y * hit.t;

      /* Reflect with slight roughness */
      var dot2 = 2 * (dir.x * hit.nx + dir.y * hit.ny);
      var rx = dir.x - dot2 * hit.nx + rand(-0.08, 0.08);
      var ry = dir.y - dot2 * hit.ny + rand(-0.08, 0.08);
      var rl = Math.sqrt(rx * rx + ry * ry) || 1;
      dir = { x: rx / rl, y: ry / rl };
    }

    /* Cumulative lengths + precompute per-segment values */
    var totalLen = 0;
    for (var s = 0; s < segments.length; s++) {
      var sg = segments[s];
      var ddx = sg.x1 - sg.x0, ddy = sg.y1 - sg.y0;
      sg.len = Math.sqrt(ddx * ddx + ddy * ddy);
      sg.cumStart = totalLen;
      totalLen += sg.len;
      var ds = 1.0 / (1.0 + sg.depth * 2.5);
      sg.coc = cocRadius(sg.depth) * ds;
    }

    return {
      segments: segments,
      totalLen: totalLen,
      color: c,
      born: time,
      lifetime: RAY_LIFETIME + rand(-2000, 2000),
      alpha: rand(0.16, 0.35),
    };
  }

  /* -- Coordinate helpers (aspect-ratio-locked) ----------------- */
  function toScreenX(nx) { return offX + nx * sceneW; }
  function toScreenY(ny) { return offY + ny * sceneH; }

  /* -- Resize --------------------------------------------------- */
  function resize() {
    dpr = window.devicePixelRatio || 1;
    var rect = canvas.parentElement.getBoundingClientRect();
    width  = rect.width;
    height = rect.height;
    canvas.width  = width * dpr;
    canvas.height = height * dpr;
    canvas.style.width  = width + 'px';
    canvas.style.height = height + 'px';
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

    /* Cover-fit: fill the container, crop overflow, never stretch */
    var containerAspect = width / height;
    if (containerAspect > SCENE_ASPECT) {
      /* Container is wider than scene — match width, crop top/bottom */
      sceneW = width;
      sceneH = width / SCENE_ASPECT;
      offX = 0;
      offY = (height - sceneH) * 0.5;
    } else {
      /* Container is taller — match height, crop left/right */
      sceneH = height;
      sceneW = height * SCENE_ASPECT;
      offX = (width - sceneW) * 0.5;
      offY = 0;
    }

    /* Precompute screen-space coords for scene segments */
    for (var i = 0; i < sceneSegs.length; i++) {
      var s = sceneSegs[i];
      s.sx0 = offX + s.x0 * sceneW;
      s.sy0 = offY + s.y0 * sceneH;
      s.sx1 = offX + s.x1 * sceneW;
      s.sy1 = offY + s.y1 * sceneH;
    }
  }

  function rgba(c, a) {
    return 'rgba(' + c[0] + ',' + c[1] + ',' + c[2] + ',' + a + ')';
  }

  /* -- Draw scene architecture (precomputed values) ------------- */
  function drawSceneArch(t) {
    var pulse = SCENE_ALPHA * (0.7 + 0.3 * Math.sin(t * 0.0003));
    ctx.lineCap = 'round';
    for (var si = 0; si < sceneSegs.length; si++) {
      var s = sceneSegs[si];
      if (s.depthOcc < 0.015) continue;
      /* Viewport culling using precomputed screen coords */
      if ((s.sx0 < -20 && s.sx1 < -20) || (s.sx0 > width + 20 && s.sx1 > width + 20)) continue;
      if ((s.sy0 < -20 && s.sy1 < -20) || (s.sy0 > height + 20 && s.sy1 > height + 20)) continue;
      var a = pulse * s.depthOcc;
      ctx.beginPath();
      ctx.moveTo(s.sx0, s.sy0);
      ctx.lineTo(s.sx1, s.sy1);
      ctx.strokeStyle = rgba(s.baseCol, a * (s.isColored ? 1.5 : 1.0));
      ctx.lineWidth = s.baseLw * (s.isColored ? 1.3 : 1.0);
      ctx.stroke();
    }
  }

  /* -- Main draw loop ------------------------------------------- */
  function draw(time) {
    raf = requestAnimationFrame(draw);
    var t = time || 0;
    ctx.clearRect(0, 0, width, height);

    drawSceneArch(t);

    /* Spawn */
    if (t >= nextSpawnTime && rays.length < RAY_COUNT) {
      rays.push(buildRay(t));
      nextSpawnTime = t + SPAWN_STAGGER + rand(-400, 400);
    }

    /* Process rays */
    for (var ri = rays.length - 1; ri >= 0; ri--) {
      var ray = rays[ri];
      var age = t - ray.born;
      if (age > ray.lifetime) { rays.splice(ri, 1); continue; }

      var lifeNorm = age / ray.lifetime;
      var envelope;
      if (lifeNorm < 0.08) envelope = lifeNorm / 0.08;
      else if (lifeNorm > 0.75) envelope = (1 - lifeNorm) / 0.25;
      else envelope = 1;
      envelope = clamp01(envelope);

      var travelStart = ray.lifetime * 0.03;
      var travelEnd   = ray.lifetime * 0.58;
      var photonProg  = clamp01((age - travelStart) / (travelEnd - travelStart));
      var drawnLen    = photonProg * ray.totalLen;
      var tailLen     = ray.totalLen * TAIL_FRACTION;
      var tailStart   = Math.max(0, drawnLen - tailLen);
      var baseAlpha   = ray.alpha * envelope;

      /* Draw ray as continuous straight-line path with fading tail */
      for (var si = 0; si < ray.segments.length; si++) {
        var seg = ray.segments[si];
        var segEnd = seg.cumStart + seg.len;
        if (seg.cumStart >= drawnLen) break;
        if (segEnd <= tailStart) continue;

        /* Clamp visible portion to tail..head window */
        var clipS = clamp01((tailStart - seg.cumStart) / seg.len);
        var clipE = clamp01((drawnLen - seg.cumStart) / seg.len);

        var sx0 = toScreenX(seg.x0 + (seg.x1 - seg.x0) * clipS);
        var sy0 = toScreenY(seg.y0 + (seg.y1 - seg.y0) * clipS);
        var sx1 = toScreenX(seg.x0 + (seg.x1 - seg.x0) * clipE);
        var sy1 = toScreenY(seg.y0 + (seg.y1 - seg.y0) * clipE);

        /* Viewport cull */
        if ((sx0 < -30 && sx1 < -30) || (sx0 > width + 30 && sx1 > width + 30)) continue;
        if ((sy0 < -30 && sy1 < -30) || (sy0 > height + 30 && sy1 > height + 30)) continue;

        /* Per-segment alpha based on distance from head + depth fade */
        var segMidDist = seg.cumStart + seg.len * (clipS + clipE) * 0.5;
        var headDist = clamp01((drawnLen - segMidDist) / tailLen);
        var tailA = 1 - headDist;
        tailA *= tailA;
        var depthFade = Math.exp(-seg.depth * 2.4);
        var a = baseAlpha * tailA * depthFade;

        if (a > 0.002) {
          /* Solid-colour layers — no gradient allocation */
          var coc = seg.coc;
          ctx.lineCap = 'round';

          ctx.beginPath(); ctx.moveTo(sx0, sy0); ctx.lineTo(sx1, sy1);
          ctx.strokeStyle = rgba(ray.color, a * 0.04);
          ctx.lineWidth = coc * 2.2; ctx.stroke();

          ctx.beginPath(); ctx.moveTo(sx0, sy0); ctx.lineTo(sx1, sy1);
          ctx.strokeStyle = rgba(ray.color, a * 0.10);
          ctx.lineWidth = coc * 1.2; ctx.stroke();

          ctx.beginPath(); ctx.moveTo(sx0, sy0); ctx.lineTo(sx1, sy1);
          ctx.strokeStyle = rgba(ray.color, a * 0.28);
          ctx.lineWidth = Math.max(1, coc * 0.3); ctx.stroke();

          ctx.beginPath(); ctx.moveTo(sx0, sy0); ctx.lineTo(sx1, sy1);
          ctx.strokeStyle = rgba(warmWhite, a * 0.40);
          ctx.lineWidth = Math.max(0.5, coc * 0.08); ctx.stroke();
        }
      }

      /* Photon glow */
      if (photonProg > 0.01 && photonProg < 0.99) {
        var pDist = drawnLen;
        for (var pi = 0; pi < ray.segments.length; pi++) {
          var pseg = ray.segments[pi];
          if (pDist <= pseg.len) {
            var pr = pDist / pseg.len;
            var px = toScreenX(pseg.x0 + (pseg.x1 - pseg.x0) * pr);
            var py = toScreenY(pseg.y0 + (pseg.y1 - pseg.y0) * pr);
            var pCoc = pseg.coc * 1.4;
            var pDepthFade = Math.exp(-pseg.depth * 2.4);
            var pAlpha = baseAlpha * pDepthFade;

            var pgGrad = ctx.createRadialGradient(px, py, 0, px, py, pCoc);
            pgGrad.addColorStop(0,    rgba(warmWhite, pAlpha * 0.60));
            pgGrad.addColorStop(0.20, rgba(ray.color, pAlpha * 0.28));
            pgGrad.addColorStop(0.55, rgba(ray.color, pAlpha * 0.06));
            pgGrad.addColorStop(1,    rgba(ray.color, 0));
            ctx.beginPath();
            ctx.arc(px, py, pCoc, 0, Math.PI * 2);
            ctx.fillStyle = pgGrad;
            ctx.fill();
            break;
          }
          pDist -= pseg.len;
        }
      }
    }
  }

  /* -- Init ----------------------------------------------------- */
  buildScene();
  resize();
  window.addEventListener('resize', resize);
  for (var k = 0; k < 14; k++) {
    rays.push(buildRay(-k * 1800));
  }
  nextSpawnTime = 0;
  raf = requestAnimationFrame(draw);

  document.addEventListener('visibilitychange', function () {
    if (document.hidden) { cancelAnimationFrame(raf); }
    else { raf = requestAnimationFrame(draw); }
  });
})();
