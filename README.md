# Drop of Life

_Giving blood saves lives._

Drop of Life is a desktop display that shows when you are eligible to donate so you can give the gift of life.

![Drop](Photos/drop%20full.jpg)

### Read the full [project writeup on Hackster](https://www.hackster.io/monkbroc/drop-of-life-e2be83)

### API

log in:
```
curl 'https://www.redcrossblood.org/api/auth/v1/login' -H 'content-type: application/json' --data-binary '{"username": "xxxxxxx","password": "xxxxxxxxxx","provider": "RCB","stayLoggedIn": true}'
```

profile:
```
curl 'https://www.redcrossblood.org/api/drive/v1/profile' -H 'cookie: RCB_AUTH=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
```

## License
Copyright 2017 Julien Vanier

Released under the MIT license
