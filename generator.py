import random
import string

def generate_random_string_with_id(num_sets=5, string_length=10):
    """
    Generate random strings with unique IDs.
    
    Args:
        num_sets: Number of sets to generate
        string_length: Length of each random string
    """
    used_ids = set()
    
    for i in range(num_sets):
        # Generate random string
        random_str = ''.join(random.choices(string.ascii_letters + string.digits, k=string_length))
        
        # Generate unique ID
        while True:
            unique_id = random.randint(1, 1000000)
            if unique_id not in used_ids:
                used_ids.add(unique_id)
                break
        
        print(random_str)
        print(unique_id)

# Generate 5 random strings with unique IDs
generate_random_string_with_id(num_sets=5000)