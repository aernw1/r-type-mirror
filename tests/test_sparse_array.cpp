#include "ECS/SparseArray.hpp"
#include "ECS/Component.hpp"
#include <iostream>
#include <cassert>

using namespace RType::ECS;

void test_basic_operations() {
    std::cout << "=== Test: Basic Operations ===" << std::endl;
    
    SparseArray<Position> positions;
    
    // Test insert_at
    positions.insert_at(0, Position{10.0f, 20.0f});
    positions.insert_at(2, Position{30.0f, 40.0f});
    positions.insert_at(5, Position{50.0f, 60.0f});
    
    assert(positions.size() == 6);
    std::cout << "insert_at works" << std::endl;
    
    // Test operator[]
    auto& pos0 = positions[0];
    assert(pos0.has_value());
    assert(pos0.value().x == 10.0f);
    assert(pos0.value().y == 20.0f);
    std::cout << "operator[] access works" << std::endl;
    
    // Test emplace_at
    positions.emplace_at(7, 70.0f, 80.0f);
    assert(positions.size() == 8);
    assert(positions[7].has_value());
    assert(positions[7].value().x == 70.0f);
    std::cout << "emplace_at works" << std::endl;
    
    // Test holes (indices without components)
    assert(!positions[1].has_value());
    assert(!positions[3].has_value());
    std::cout << "Holes work correctly" << std::endl;
    
    // Test erase
    positions.erase(2);
    assert(!positions[2].has_value());
    assert(positions.size() == 8); // Size doesn't decrease
    std::cout << "erase works" << std::endl;
    
    std::cout << "Basic operations: PASSED\n" << std::endl;
}

void test_iteration() {
    std::cout << "=== Test: Iteration ===" << std::endl;
    
    SparseArray<Position> positions;
    positions.insert_at(0, Position{1.0f, 2.0f});
    positions.insert_at(2, Position{3.0f, 4.0f});
    positions.insert_at(4, Position{5.0f, 6.0f});
    
    int count = 0;
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i].has_value()) {
            count++;
        }
    }
    assert(count == 3);
    std::cout << "Manual iteration works" << std::endl;
    
    // Test range-based for
    count = 0;
    for (auto& opt : positions) {
        if (opt.has_value()) {
            count++;
        }
    }
    assert(count == 3);
    std::cout << "Range-based for works" << std::endl;
    
    std::cout << "Iteration: PASSED\n" << std::endl;
}

void test_copy_move() {
    std::cout << "=== Test: Copy and Move ===" << std::endl;
    
    SparseArray<Position> original;
    original.insert_at(0, Position{10.0f, 20.0f});
    original.insert_at(2, Position{30.0f, 40.0f});
    
    // Test copy
    SparseArray<Position> copied = original;
    assert(copied.size() == original.size());
    assert(copied[0].has_value());
    assert(copied[0].value().x == 10.0f);
    std::cout << "Copy constructor works" << std::endl;
    
    // Test move
    SparseArray<Position> moved = std::move(original);
    assert(moved.size() == 3);
    assert(moved[0].has_value());
    assert(original.size() == 0 || original.size() == 0); // Moved from
    std::cout << "Move constructor works" << std::endl;
    
    std::cout << "Copy and Move: PASSED\n" << std::endl;
}

void test_get_index() {
    std::cout << "=== Test: get_index ===" << std::endl;
    
    SparseArray<Position> positions;
    positions.insert_at(5, Position{10.0f, 20.0f});
    
    auto& pos_opt = positions[5];
    size_t idx = positions.get_index(pos_opt);
    assert(idx == 5);
    std::cout << "get_index works" << std::endl;
    
    std::cout << "get_index: PASSED\n" << std::endl;
}

void test_velocity_component() {
    std::cout << "=== Test: Velocity Component ===" << std::endl;
    
    SparseArray<Velocity> velocities;
    velocities.emplace_at(0, 1.0f, 2.0f);
    velocities.emplace_at(3, 3.0f, 4.0f);
    
    assert(velocities[0].has_value());
    assert(velocities[0].value().dx == 1.0f);
    assert(velocities[0].value().dy == 2.0f);
    std::cout << "Works with Velocity component" << std::endl;
    
    std::cout << "Velocity Component: PASSED\n" << std::endl;
}

int main() {
    std::cout << "Testing SparseArray...\n" << std::endl;
    
    try {
        test_basic_operations();
        test_iteration();
        test_copy_move();
        test_get_index();
        test_velocity_component();
        
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test FAILED: " << e.what() << std::endl;
        return 1;
    }
}

