#!/usr/bin/env bash

set -e

export QQBOT_APP_ID='你的 AppID'
export QQBOT_CLIENT_SECRET='你的 ClientSecret'

exec ./build/qqbot
