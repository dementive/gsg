#pragma once

#include "core/templates/a_hash_map.h"
#include "core/templates/hash_set.h"
#include "core/variant/variant.h"

namespace CG {

// 2D AStar pathfinding. This is the exact same as godot's AStar2D class in core/math/a_star.h except it isn't an object and doesn't wrap the 3D pathfinding so uses 10x less memory.
class AStar {
	struct Point {
		Point() = default;

		int64_t id = 0;
		Vector2 pos;
		real_t weight_scale = 0;
		bool enabled = false;

		AHashMap<int64_t, Point *> neighbors = 4U;
		AHashMap<int64_t, Point *> unlinked_neighbours = 4U;

		// Used for pathfinding.
		Point *prev_point = nullptr;
		real_t g_score = 0;
		real_t f_score = 0;
		uint64_t open_pass = 0;
		uint64_t closed_pass = 0;

		// Used for getting closest_point_of_last_pathing_call.
		real_t abs_g_score = 0;
		real_t abs_f_score = 0;
	};

	struct SortPoints {
		_FORCE_INLINE_ bool operator()(const Point *A, const Point *B) const { // Returns true when the Point A is worse than Point B.
			if (A->f_score > B->f_score)
				return true;
			else if (A->f_score < B->f_score)
				return false;
			else
				return A->g_score < B->g_score; // If the f_costs are the same then prioritize the points that are further away from the start.
		}
	};

	struct Segment {
		Pair<int64_t, int64_t> key;

		enum : uint8_t { NONE = 0, FORWARD = 1, BACKWARD = 2, BIDIRECTIONAL = FORWARD | BACKWARD };
		unsigned char direction = NONE;

		static uint32_t hash(const Segment &p_seg) { return HashMapHasherDefault::hash(p_seg.key); }
		bool operator==(const Segment &p_s) const { return key == p_s.key; }

		Segment() = default;
		Segment(int64_t p_from, int64_t p_to) {
			if (p_from < p_to) {
				key.first = p_from;
				key.second = p_to;
				direction = FORWARD;
			} else {
				key.first = p_to;
				key.second = p_from;
				direction = BACKWARD;
			}
		}
	};

	mutable int64_t last_free_id = 0;
	uint64_t pass = 1;

	AHashMap<int64_t, Point *> points;
	HashSet<Segment, Segment> segments;
	Point *last_closest_point = nullptr;

	bool _solve(Point *begin_point, Point *end_point, bool p_allow_partial_path);

protected:
	static void _bind_methods();

	real_t _estimate_cost(int64_t p_from_id, int64_t p_end_id);
	real_t _compute_cost(int64_t p_from_id, int64_t p_to_id);

public:
	void add_point(int64_t p_id, const Vector2 &p_pos, real_t p_weight_scale = 1);
	Vector2 get_point_position(int64_t p_id) const;
	void set_point_position(int64_t p_id, const Vector2 &p_pos);
	void remove_point(int64_t p_id);
	bool has_point(int64_t p_id) const;
	Vector<int64_t> get_point_connections(int64_t p_id);
	PackedInt64Array get_point_ids();

	void set_point_disabled(int64_t p_id, bool p_disabled = true);
	bool is_point_disabled(int64_t p_id) const;

	void connect_points(int64_t p_id, int64_t p_with_id, bool bidirectional = true);
	void disconnect_points(int64_t p_id, int64_t p_with_id, bool bidirectional = true);
	bool are_points_connected(int64_t p_id, int64_t p_with_id, bool bidirectional = true) const;

	int64_t get_point_count() const;
	int64_t get_point_capacity() const;
	void reserve_space(int64_t p_num_nodes);
	void clear();

	int64_t get_closest_point(const Vector2 &p_point, bool p_include_disabled = false) const;
	Vector2 get_closest_position_in_segment(const Vector2 &p_point) const;

	Vector<Vector2> get_point_path(int64_t p_from_id, int64_t p_to_id, bool p_allow_partial_path = false);
	Vector<int64_t> get_id_path(int64_t p_from_id, int64_t p_to_id, bool p_allow_partial_path = false);

	AStar() = default;
	~AStar();
};

} // namespace CG
