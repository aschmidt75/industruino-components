package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
)

func ComputeHmac256(message string, key []byte) string {
	h := hmac.New(sha256.New, key)
	h.Write([]byte(message))
	return hex.EncodeToString(h.Sum(nil))
}

func main() {
	key := make([]uint8, 32)
	key_phrase := "This is a key phrase."
	const num_rounds = 10

	// derive key from key_phrase by round hashing it
	copy(key, []uint8(key_phrase))
	for i := 0; i < num_rounds; i++ {
		h := sha256.New()
		h.Write(key)
		new_key := sha256.Sum256(key)
		copy(key, []uint8(new_key[:]))
	}

	data := "This is something to hash."

	fmt.Println(ComputeHmac256(data, key))
}
