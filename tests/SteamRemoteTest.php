<?php

use PHPUnit\Framework\TestCase;

class SteamRemoteTest extends TestCase
{
    public function testFileWriteExists(): void
    {
        $this->assertTrue(function_exists('steam_remote_file_write'));
    }

    public function testFileReadExists(): void
    {
        $this->assertTrue(function_exists('steam_remote_file_read'));
    }

    public function testFileExistsExists(): void
    {
        $this->assertTrue(function_exists('steam_remote_file_exists'));
    }

    public function testFileDeleteExists(): void
    {
        $this->assertTrue(function_exists('steam_remote_file_delete'));
    }

    public function testFileListExists(): void
    {
        $this->assertTrue(function_exists('steam_remote_file_list'));
    }
}
