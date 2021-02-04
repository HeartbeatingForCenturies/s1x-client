#include <std_include.hpp>
#include "demonware.hpp"
#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace demonware
{
    // SHA256 auth signature, Hmacs ...
    char dw_key[296] = "\x30\x82\x01\x22\x30\x0D\x06\x09\x2A\x86\x48\x86\xF7\x0D\x01\x01"
        "\x01\x05\x00\x03\x82\x01\x0F\x00\x30\x82\x01\x0A\x02\x82\x01\x01"
        "\x00\xC0\xA2\x0B\x1F\x6C\xB8\x1B\x12\x70\xED\x1A\xEF\x30\x6C\x75"
        "\x9D\xC1\x08\x89\x99\xF0\x2A\xC8\xAC\x2F\xC7\xD5\xD0\x3B\x61\x29"
        "\x39\xF3\x8F\x62\x39\xDA\xF1\x20\x11\xE7\x92\xE9\x16\x24\x22\x96"
        "\x09\x9E\xAC\x19\xCD\x24\x3E\x58\xC6\x40\x86\x78\xD7\xDF\x70\x77"
        "\xCB\xDE\x80\x42\xB1\x38\xF3\x1D\x6A\x3C\x98\xE4\x85\xDB\xFB\x53"
        "\x3A\x86\x47\xCE\x58\xB1\xD3\xD7\x0B\x83\x3D\x14\x6B\xDA\x40\x24"
        "\x1F\x16\x2B\x0E\x49\x22\xE4\xB7\x63\xFF\xAA\x40\xC2\x44\xDF\xDC"
        "\x3F\x8C\x1E\x60\xB4\x6F\x3E\xDA\xB2\x4E\x50\xCA\xFC\x62\x4B\x62"
        "\xC7\xE1\x77\x5E\x83\xCD\xE0\xB5\xFC\xC6\xAA\xA0\xC2\x6B\x28\xCC"
        "\x8A\xA7\x95\x7B\x1E\x67\xE0\x5B\xAF\xC6\x54\x49\xE6\xAC\x7A\x8D"
        "\x1D\xE6\x7D\x12\x04\x94\xC3\x23\x4A\x00\x60\x58\x33\x6F\xE7\x94"
        "\x19\xFF\xF6\xE0\xC6\x40\x50\xB7\x9D\x0E\xCD\xDF\xE7\x92\x5D\x84"
        "\x94\x13\x06\x61\xBC\x44\x75\x54\x70\x54\x77\x4C\xC0\x28\x7D\xFC"
        "\xC9\x9A\x92\x38\xD4\xD5\xEE\xF3\x27\x44\x66\x13\x2C\x06\xF0\x64"
        "\xE7\xEC\xF8\x75\xFD\x15\xD4\x1B\x91\x45\x9D\x4A\x3F\x40\xE9\x35"
        "\x53\x7F\xFC\x96\x61\xE1\x48\x74\x21\xF0\x04\x20\x41\x30\x02\xD2"
        "\xF9\x02\x03\x01\x00\x01\x00";

    struct data_t
    {
        char m_session_key[24];
        char m_response[8];
        char m_hmac_key[20];
        char m_enc_key[16];
        char m_dec_key[16];
    }data{};

    std::string packet_buffer;

    bool calculate_hmacs(const char* input, unsigned int inputSize, const char* key, unsigned int keySize, void* dst, unsigned int dstSize)
    {
        char buffer[400];
        unsigned int pos = 0;
        unsigned int out_offset = 0;
        char count = 1;
        char result[20];

        std::memcpy(&buffer[pos], key, keySize);
        pos += keySize;

        buffer[pos] = count;
        pos++;

        unsigned int outlen = 20;
        std::string output = utils::cryptography::hmac_sha1::process(std::string(buffer, pos), std::string(input, inputSize), &outlen);

        std::memcpy(result, output.data(), 20);
        std::memcpy(dst, result, 20);
        out_offset = 20;
        // second loop
        while (1)
        {
            if (out_offset >= dstSize)
                return true;

            pos = 0;
            std::memcpy(&buffer[pos], result, 20);
            pos += 20;

            std::memcpy(&buffer[pos], key, keySize);
            pos += keySize;

            count++;
            buffer[pos] = count;
            pos++;

            unsigned int outlen2 = 20;
            std::string output2 = utils::cryptography::hmac_sha1::process(std::string(buffer, pos), std::string(input, inputSize), &outlen2);
            std::memcpy(result, output2.data(), 20);
            std::memcpy(&((char*)dst)[out_offset], result, std::min(20u, (dstSize - out_offset)));
            out_offset += 20;
        }
        return true;
    }

    void derive_keys()
    {
        std::string key_1 = utils::cryptography::sha1::compute(packet_buffer);

        char out_1[24];
        calculate_hmacs(data.m_session_key, 24, dw_key, 294, out_1, 24);

        unsigned int len = 20;
        std::string data_2(out_1, 24);
        std::string data_3 = utils::cryptography::hmac_sha1::process(data_2, key_1, &len);

        char out_2[16];
        calculate_hmacs(data_3.data(), 20, "CLIENTCHAL", 10, out_2, 16);

        char out_3[72];
        calculate_hmacs(data_3.data(), 20, "BDDATA", 6, out_3, 72);

        std::memcpy(data.m_response, &out_2[8], 8);
        std::memcpy(data.m_dec_key, &out_3[40], 16);
        std::memcpy(data.m_enc_key, &out_3[56], 16);

#ifdef DEBUG
        printf("[demonware] HmacSHA1 id: %s\n", utils::string::dump_hex(std::string(&out_2[8], 8)).data());
        printf("[demonware] AES enc key: %s\n", utils::string::dump_hex(std::string(&out_3[40], 16)).data());
        printf("[demonware] AES dec key: %s\n", utils::string::dump_hex(std::string(&out_3[56], 16)).data());
        printf("[demonware] Bravo 6, going dark.\n");
#endif
    }

    void calculate_hmacs_s1(const char* data_, unsigned int data_size, const char* key, unsigned int key_size, char* dst, unsigned int dst_size)
    {
        char buffer[64];
        unsigned int pos = 0;
        unsigned int out_offset = 0;
        char count = 1;
        std::string result;

        // buffer add key
        std::memcpy(&buffer[pos], key, key_size);
        pos += key_size;

        // buffer add count
        buffer[pos] = count;
        pos++;

        // calculate hmac
        unsigned int outlen = 20;
        result = utils::cryptography::hmac_sha1::process(std::string(buffer, pos), std::string(data_, data_size), &outlen);

        // save output
        std::memcpy(dst, result.data(), std::min(20u, (dst_size - out_offset)));
        out_offset = 20;

        // second loop
        while (1)
        {
            // if we filled the output buffer, exit
            if (out_offset >= dst_size)
                break;

            // buffer add last result
            pos = 0;
            std::memcpy(&buffer[pos], result.data(), 20);
            pos += 20;

            // buffer add key
            std::memcpy(&buffer[pos], key, key_size);
            pos += key_size;

            // buffer add count
            count++;
            buffer[pos] = count;
            pos++;

            // calculate hmac
            unsigned int outlen_ = 20;
            result = utils::cryptography::hmac_sha1::process(std::string(buffer, pos), std::string(data_, data_size), &outlen_);

            // save output
            std::memcpy(&((char*)dst)[out_offset], result.data(), std::min(20u, (dst_size - out_offset)));
            out_offset += 20;
        }
    }

    void derive_keys_s1()
    {
        std::string out_1 = utils::cryptography::sha1::compute(packet_buffer); // out_1 size 20

        unsigned int len = 20;
        std::string data_3 = utils::cryptography::hmac_sha1::process(data.m_session_key, out_1, &len);

        char out_2[16];
        calculate_hmacs_s1(data_3.data(), 20, "CLIENTCHAL", 10, out_2, 16);

        char out_3[72];
        calculate_hmacs_s1(data_3.data(), 20, "BDDATA", 6, out_3, 72);

        std::memcpy(data.m_response, &out_2[8], 8);
        std::memcpy(data.m_hmac_key, &out_3[20], 20);
        std::memcpy(data.m_dec_key, &out_3[40], 16);
        std::memcpy(data.m_enc_key, &out_3[56], 16);

#ifdef DEBUG
        printf("[demonware] Response id: %s\n", utils::string::dump_hex(std::string(&out_2[8], 8)).data());
        printf("[demonware] Hash verify: %s\n", utils::string::dump_hex(std::string(&out_3[20], 20)).data());
        printf("[demonware] AES dec key: %s\n", utils::string::dump_hex(std::string(&out_3[40], 16)).data());
        printf("[demonware] AES enc key: %s\n", utils::string::dump_hex(std::string(&out_3[56], 16)).data());
        printf("[demonware] Bravo 6, going dark.\n");
#endif
    }

    void queue_packet_to_hash(const std::string& packet)
    {
        packet_buffer.append(packet);
    }

    void set_session_key(const std::string& key)
    {
        std::memcpy(data.m_session_key, key.data(), 24);
    }

    std::string get_decrypt_key()
    {
        return std::string(data.m_dec_key, 16);
    }

    std::string get_encrypt_key()
    {
        return std::string(data.m_enc_key, 16);
    }

    std::string get_hmac_key()
    {
        return std::string(data.m_hmac_key, 20);
    }

    std::string get_response_id()
    {
        return std::string(data.m_response, 8);
    }
}
