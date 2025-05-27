#!/usr/bin/env python3

import os
import sys

try:
    import gi
    gi.require_version('Secret', '1')
    from gi.repository import Secret
    
    # Test if we can access the secret service
    service = Secret.Service.get_sync(Secret.ServiceFlags.NONE, None)
    if service:
        print("✅ GNOME Keyring is working correctly!")
        print("✅ Secret service is accessible")
        
        # Try to get collections
        collections = service.get_collections()
        print(f"✅ Found {len(collections)} keyring collections")
        
        sys.exit(0)
    else:
        print("❌ Could not connect to secret service")
        sys.exit(1)
        
except Exception as e:
    print(f"❌ Error accessing keyring: {e}")
    print("This might indicate the keyring is not properly configured")
    sys.exit(1)
