#define PLAYER_SPEED 0.7f
#define ROTATION_SPEED 1.5f
#define DELTA 0.00001f
#define Z_FRACTION_BITS 16
#define NEAR_PLANE 0.20f
#define FAR_PLANE 16.0f
#define INV_NEAR_PLANE ((1 << Z_FRACTION_BITS) * (1.0f/NEAR_PLANE))
#define INV_FAR_PLANE ((1 << Z_FRACTION_BITS) * (1.0f/FAR_PLANE))

typedef struct Vec2 Vec2;
struct Vec2 {
	f32 x, y;
};

typedef struct Vec3 Vec3;
struct Vec3 {
	f32 x, y, z;
};

typedef struct Face Face;
struct Face {
	u32 v1, v2, v3;
};

typedef struct Mesh Mesh;
struct Mesh {
	Vec3 *vert;
	u32 vert_count;

	Face* faces;
	u32 faces_count;
};

typedef struct LinearTransform LinearTransform;
struct LinearTransform {
	f32 ax, bx, cx, dx;
	f32 ay, by, cy, dy;
	f32 az, bz, cz, dz;
};

typedef struct MeshInstance MeshInstance;
struct MeshInstance {
	Mesh mesh;
	LinearTransform transform;
};

typedef struct Control Control;
struct Control {
	u8 w, s, a, d;
	u8 q, e;
	u8 u, i;
};

typedef struct Player Player;
struct Player {
	Vec3 pos;
	Vec3 rot;
};

typedef struct GameState GameState;
struct GameState {
	Player  player;
	Control control;
	MeshInstance mesh_instance;
};

#define proper_modf(a, b) (modf32((modf32(a, b) + b), b))

f32 modf32(f32 a, f32 b)
{
	i32 d = a / b;
	return a - b*d;
}



f32 linear_transform_op(f32 a, f32 b, f32 c, f32 d, Vec3 v) 
{
	return a*v.x + b*v.y + c*v.z + d;
}


LinearTransform linear_transform_mul(LinearTransform u, LinearTransform z) 
{
	return (LinearTransform){
		linear_transform_op(u.ax, u.bx, u.cx, 0.0f, (Vec3){z.ax, z.ay, z.az}),
		linear_transform_op(u.ax, u.bx, u.cx, 0.0f, (Vec3){z.bx, z.by, z.bz}),
		linear_transform_op(u.ax, u.bx, u.cx, 0.0f, (Vec3){z.cx, z.cy, z.cz}),
		linear_transform_op(u.ax, u.bx, u.cx, u.dx, (Vec3){z.dx, z.dy, z.dz}),
                                                                     
		linear_transform_op(u.ay, u.by, u.cy, 0.0f, (Vec3){z.ax, z.ay, z.az}),
		linear_transform_op(u.ay, u.by, u.cy, 0.0f, (Vec3){z.bx, z.by, z.bz}),
		linear_transform_op(u.ay, u.by, u.cy, 0.0f, (Vec3){z.cx, z.cy, z.cz}),
		linear_transform_op(u.ay, u.by, u.cy, u.dy, (Vec3){z.dx, z.dy, z.dz}),
                                                                     
		linear_transform_op(u.az, u.bz, u.cz, 0.0f, (Vec3){z.ax, z.ay, z.az}),
		linear_transform_op(u.az, u.bz, u.cz, 0.0f, (Vec3){z.bx, z.by, z.bz}),
		linear_transform_op(u.az, u.bz, u.cz, 0.0f, (Vec3){z.cx, z.cy, z.cz}),
		linear_transform_op(u.az, u.bz, u.cz, u.dz, (Vec3){z.dx, z.dy, z.dz})
	};
}

LinearTransform linear_transform_identity()
{
	return (LinearTransform){
		1.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 1.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f
	};
}

Vec3 linear_transform_apply(LinearTransform u, Vec3 z) 
{
	return (Vec3){
		linear_transform_op(u.ax, u.bx, u.cx, u.dx, z),
		linear_transform_op(u.ay, u.by, u.cy, u.dy, z),
		linear_transform_op(u.az, u.bz, u.cz, u.dz, z)
	};
}

MeshInstance mesh_create_instance(const Mesh mesh) {
	return (MeshInstance){mesh, linear_transform_identity()};
}

i32 shift_right_round(i64 a, i32 b) 
{
	return (a + ((1ll << b) -1)) >> b;
}

void draw_triangle(Vec2 v1, Vec2 v2, Vec2 v3, 
				   f32 fz1, f32 fz2, f32 fz3,
				   PixelType color,
				   i32 *zbuffer,
                   canvas_ptr canvas)
{
	// Ordering v1 v2 v3 in counter clockwise
	if ((v2.y - v1.y) * (v3.x - v2.x) - (v2.x - v1.x) * (v3.y - v2.y) > 0) {
		SWAP(Vec2, v2, v3); 
		SWAP(f32, fz2, fz3);
	}

	// Xs and Ys * 16
	i32 y1 = v1.y * 16.0f;
	i32 y2 = v2.y * 16.0f;
	i32 y3 = v3.y * 16.0f;

	i32 x1 = v1.x * 16.0f;
	i32 x2 = v2.x * 16.0f;
	i32 x3 = v3.x * 16.0f;

	i32 z1 = fz1 * (1 << Z_FRACTION_BITS);
	i32 z2 = fz2 * (1 << Z_FRACTION_BITS);
	i32 z3 = fz3 * (1 << Z_FRACTION_BITS);

	if ((z1 <= 0 || z1 > INV_NEAR_PLANE) && 
	 	(z2 <= 0 || z2 > INV_NEAR_PLANE) && 
	 	(z3 <= 0 || z3 > INV_NEAR_PLANE)) return;

	// Step functions *16
	i32 dx21 = x2 - x1;
	i32 dx32 = x3 - x2;
	i32 dx13 = x1 - x3;
	i32 dx31 = x3 - x1;

	i32 dy21 = y2 - y1;
	i32 dy32 = y3 - y2;
	i32 dy13 = y1 - y3;
	i32 dy31 = y3 - y1;

	i32 dz13 = z1 - z3;
	i32 dz23 = z2 - z3;

	// Step functions * 16 * 16
	i32 fdx21 = dx21 << 4;
	i32 fdx32 = dx32 << 4;
	i32 fdx13 = dx13 << 4;

	i32 fdy21 = dy21 << 4;
	i32 fdy32 = dy32 << 4;
	i32 fdy13 = dy13 << 4;

	// minx, miny, maxx, maxy
	i32 minxf = min3(x1, x2, x3);
	i32 minx =  max((minxf + 0xF) >> 4, 0);

	i32 minyf = min3(y1, y2, y3);
	i32 miny =  max((minyf + 0xF) >> 4, 0);
	
	i32 maxxf = max3(x1, x2, x3);
	i32 maxx =  min((maxxf + 0xF) >> 4, WINDOW_WIDTH);

	i32 maxyf = max3(y1, y2, y3);
	i32 maxy = min((maxyf + 0xF) >> 4, WINDOW_HEIGHT);

	canvas_ptr buffer = canvas + miny * WINDOW_STRIDE;
	i32 *zbufy = zbuffer + miny * WINDOW_WIDTH;

	// Equations 24.8
	i32 eq1y = dx21*((miny << 4) - y1) - dy21 * ((minx << 4) - x1);
	i32 eq2y = dx32*((miny << 4) - y2) - dy32 * ((minx << 4) - x2);
	i32 eq3y = dx13*((miny << 4) - y3) - dy13 * ((minx << 4) - x3);

	i32 area = dx21*dy31 - dy21 * dx31; // 24.8
	if (area == 0) area = 1;

	i64 fdzxt =  ((-(i64)dz13*dy32 - (i64)dz23*dy13) << 16)/area;
	i64 fdzyt =  (( (i64)dz13*dx32 + (i64)dz23*dx13) << 16)/area;

	i32 fdzx = shift_right_round(fdzxt, 12);
	i32 fdzy = shift_right_round(fdzyt, 12);

	i64 zd1 = (((i64)eq2y*dz13 + (i64)eq3y*dz23) << 16)/area;
	i32 eqz = z3 + shift_right_round(zd1, 16);

	// Correct for fill convention
    if(dy21 < 0 || (dy21 == 0 && dx21 > 0)) eq1y++;
    if(dy32 < 0 || (dy32 == 0 && dx32 > 0)) eq2y++;
    if(dy13 < 0 || (dy13 == 0 && dx13 > 0)) eq3y++;

	for (i32 y = miny; y < maxy; y++) {
		canvas_ptr xbuffer = buffer + minx;
		i32 *zb =      zbufy + minx;
		i32 eq1x = eq1y;
		i32 eq2x = eq2y;
		i32 eq3x = eq3y;
		i32 eqzx = eqz;
		for (i32 x = minx; x < maxx; x++) {
			if (eq1x > 0 &&
				eq2x > 0 &&
				eq3x > 0) {
				 if (*zb < eqzx && eqzx < INV_NEAR_PLANE) {
					*zb = eqzx;
					*xbuffer = color;
				}
			}
			xbuffer++;
			zb++;
			eq1x -= fdy21;
			eq2x -= fdy32;
			eq3x -= fdy13;
			eqzx += fdzx;
		}
		buffer += WINDOW_STRIDE;
		zbufy += WINDOW_WIDTH;
		eq1y += fdx21;
		eq2y += fdx32;
		eq3y += fdx13;
		eqz += fdzy;
	}
}

Vec2 vec3_project_to_2D(Vec3 t) {
	return (Vec2){t.x / t.z, t.y / t.z};
}

Vec2 vec2_to_screen_space(Vec2 t) {
	return (Vec2){(t.x + 1.0f) / 2.0f * WINDOW_WIDTH, 
				  (t.y + 1.0f) / 2.0f * WINDOW_HEIGHT};
}



// Project from 3D to 2D
// Determine the coordinate space
// [-1, 1] [-,1 1], [-1, 1]

PixelType colors[] = {GREEN, RED, BLUE, YELLOW, PURPLE, CYAN};
u32 colors_count = sizeof(colors) / sizeof(*colors);

// #include "data/triangle.c"
// #include "data/triangle2.c"
#include "data/tetraedro.c"
// #include "data/cup.c"

f32 t = PI/2.0f;
void draw_mesh_test(canvas_ptr canvas, i32 *zbuffer)
{

	LinearTransform translation_transform = linear_transform_identity();
	translation_transform.dz = 1.0f;

	LinearTransform rotationy_transform = {
		cosf(t),  0.0f, sinf(t), 0.0f,
		0.0f,         1.0f, 0.0f,        0.0f,
		-sinf(t), 0.0f, cosf(t), 0.0f
	};

	LinearTransform player_transform = linear_transform_mul(
		translation_transform,
		rotationy_transform
	);

	MeshInstance m = mesh_create_instance(tetraedro_mesh);
	Vec3 *v = m.mesh.vert;

	for (u32 i = 0; i < m.mesh.faces_count; i++) {
		Face f = m.mesh.faces[i];
		
		LinearTransform transform = linear_transform_mul(m.transform, player_transform);

		Vec3 tv1 = linear_transform_apply(transform, v[f.v1]);
		Vec3 tv2 = linear_transform_apply(transform, v[f.v2]);
		Vec3 tv3 = linear_transform_apply(transform, v[f.v3]);

		f32 fz1 = tv1.z;
		f32 fz2 = tv2.z;
		f32 fz3 = tv3.z;

		draw_triangle(
			vec2_to_screen_space(vec3_project_to_2D(tv1)),
			vec2_to_screen_space(vec3_project_to_2D(tv2)),
			vec2_to_screen_space(vec3_project_to_2D(tv3)),
			1.0f/fz1,
			1.0f/fz2,
			1.0f/fz3,
			colors[i%colors_count],
			zbuffer,
			canvas
	    );
	}
}

void draw_mesh(GameState *game, canvas_ptr canvas, i32 *zbuffer)
{
	LinearTransform translation_transform = linear_transform_identity();
	translation_transform.dx = -game->player.pos.x;
	translation_transform.dy = -game->player.pos.y;
	translation_transform.dz = -game->player.pos.z;

	f32 tetay = game->player.rot.y;
	f32 tetax = game->player.rot.x;

	LinearTransform rotationy_transform = {
		cosf(tetay),  0.0f, sinf(tetay), 0.0f,
		0.0f,         1.0f, 0.0f,        0.0f,
		-sinf(tetay), 0.0f, cosf(tetay), 0.0f
	};

	LinearTransform rotationx_transform = {
		1.0f, 0.0f,        0.0f,         0.0f,
		0.0f, cosf(tetax), -sinf(tetax), 0.0f,
		0.0f, sinf(tetax), cosf(tetax),  0.0f
	};


	LinearTransform player_transform = linear_transform_mul(
		translation_transform,
		linear_transform_mul(rotationy_transform, rotationx_transform)
	);

	MeshInstance m = game->mesh_instance;
	Vec3 *v = m.mesh.vert;

	for (u32 i = 0; i < m.mesh.faces_count; i++) {
		Face f = m.mesh.faces[i];
		
		LinearTransform transform = linear_transform_mul(m.transform, player_transform);

		Vec3 tv1 = linear_transform_apply(transform, v[f.v1]);
		Vec3 tv2 = linear_transform_apply(transform, v[f.v2]);
		Vec3 tv3 = linear_transform_apply(transform, v[f.v3]);

		f32 fz1 = tv1.z;
		f32 fz2 = tv2.z;
		f32 fz3 = tv3.z;

		draw_triangle(
			vec2_to_screen_space(vec3_project_to_2D(tv1)),
			vec2_to_screen_space(vec3_project_to_2D(tv2)),
			vec2_to_screen_space(vec3_project_to_2D(tv3)),
			1.0f/fz1,
			1.0f/fz2,
			1.0f/fz3,
			colors[i%colors_count],
			zbuffer,
			canvas
	    );
	}
}



void game_init(GameState *game) 
{
	game->control = (const Control){0};

	game->player.rot.x = 0.0f;
	game->player.rot.y = 0.0f;

	game->player.pos.x = 0.0f;
	game->player.pos.y = 0.0f;
	game->player.pos.z = -1.0f;
	game->mesh_instance = mesh_create_instance(tetraedro_mesh);
}

void game_update(GameState *game, f32 dt) 
{
	if (game->control.w) game->player.pos.z += PLAYER_SPEED * dt;
	if (game->control.s) game->player.pos.z -= PLAYER_SPEED * dt;
	if (game->control.d) game->player.pos.x += PLAYER_SPEED * dt;
	if (game->control.a) game->player.pos.x -= PLAYER_SPEED * dt;

	if (game->control.q) game->player.rot.y -= ROTATION_SPEED * dt;
	if (game->control.e) game->player.rot.y += ROTATION_SPEED * dt;

	if (game->control.u) game->player.rot.x -= ROTATION_SPEED * dt;
	if (game->control.i) game->player.rot.x += ROTATION_SPEED * dt;

	game->player.rot.y = proper_modf(game->player.rot.y, 2*PI);
	if (game->player.rot.y >= PI) game->player.rot.y -= 2*PI;

	game->player.rot.x = proper_modf(game->player.rot.x, 2*PI);
	if (game->player.rot.x >= PI) game->player.rot.x -= 2*PI;
}

i32 zbuffer[WINDOW_WIDTH*WINDOW_HEIGHT];
void game_draw(GameState *game, canvas_ptr canvas) 
{
	for (int i = 0; i < WINDOW_HEIGHT; i++) 
		for (int j = 0; j < WINDOW_WIDTH; j++)
			canvas[i*WINDOW_STRIDE + j] = WHITE;

	for (int i= 0; i < WINDOW_WIDTH*WINDOW_HEIGHT; i++) zbuffer[i] = INV_FAR_PLANE;

	draw_mesh(game, canvas, zbuffer);
}

void game_draw_test(canvas_ptr canvas) 
{
	for (int i = 0; i < WINDOW_HEIGHT; i++) 
		for (int j = 0; j < WINDOW_WIDTH; j++)
			canvas[i*WINDOW_STRIDE + j] = WHITE;

	for (int i= 0; i < WINDOW_WIDTH*WINDOW_HEIGHT; i++) zbuffer[i] = INV_FAR_PLANE;

	draw_mesh_test(canvas, zbuffer);
}
